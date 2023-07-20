#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libgen.h>
#include <sqlite3/sqlite3.h>
#include <dlfcn.h>

#include "gamename.h"
#include "utils/file.h"

#define MAX_FOLDER_NAME_LEN 256
#define MAX_FILE_NAME_LEN 256
#define MAX_LINE_LEN 1024
#define MAX_ROM_NAME_LENGTH 100
#define MAX_MATCHING_FOLDERS 1000
#define FULL_ROM_LIST_NAME "full-arcade-rom-name-list.txt"
#define ARCADE_ROM_NAMES_NAME "arcade-rom-names.txt"
#define MISSING_ROM_NAMES_NAME "missing_roms_name.txt"
#define ROM_NAMES_FILE_NAME "all_roms_found.txt"

#define STR_MAX 256

char matching_folders[MAX_MATCHING_FOLDERS][256];
int systems_count = 0;

//loaded shared linb function
char *(*GetGameName_func)(const char *, const char *);

typedef struct {
    char *table_name;
    sqlite3 *db;
} UpdateData;

// Remove the extension from a file name
void removeExtension(char *file_name)
{
    char *last_dot = strrchr(file_name, '.'); // Find the last dot in the file name
    if (last_dot != NULL) {
        *last_dot = '\0'; // Null-terminate the file name at the last dot
    }
}

int findFoldersWithShortname(char *disk_path, char matching_folders[][256], int i) {
    char command[STR_MAX*5];
    char path[STR_MAX*3];
    char folder[STR_MAX];
    FILE *find, *sed;

    // Use the 'find' command to search for 'config.json' files in subdirectories of the disk path
    sprintf(command, "find %s -name 'config.json' -type f", disk_path);
    find = popen(command, "r");
    if (find == NULL) {
        perror("Error executing find command");
        exit(EXIT_FAILURE);
    }

    // Read the output of the find command and extract matching folder names
    while (fgets(path, sizeof(path), find) != NULL) {
        path[strcspn(path, "\n")] = '\0'; // Remove trailing newline character

        // Check if the file contains the string '"shortname":1'
        sprintf(command, "grep -q '\"shortname\":[[:space:]]*1' '%s'", path);
        if (system(command) == 0) {
            // Get the folder name (someone could have changed the defaults)
            sprintf(command, "sed -n 's/.*\"rompath\":[[:space:]]*\"\\([^\"]*\\)\".*/\\1/p' '%s'", path);
            sed = popen(command, "r");
            if (sed == NULL) {
                perror("Error executing sed command");
                exit(EXIT_FAILURE);
            }
            fgets(folder, sizeof(folder), sed);
            folder[strcspn(folder, "\n")] = '\0'; // Remove trailing newline character
            pclose(sed);
            char * system = basename(folder);
            int cmp = -1;
            //check if we already added this folder/system
            for (int x = 0; x < i; x ++){
                cmp = strcmp(matching_folders[x], system);
                if ( cmp == 0){
                    break;
                }
            }
            if( cmp != 0){
                // Extract the folder name and add it to the matching_folders array
                sprintf(matching_folders[i], "%s", system);
                i++;
            }
            if (i == MAX_MATCHING_FOLDERS) {
                break; // Maximum number of folders reached
            }
        }
    }

    pclose(find);
    return i;
}



int sortFileLines(const char* filename) {
    char cmd[STR_MAX];
    snprintf(cmd, STR_MAX, "awk '$1' %s | sort -uk 1 -o %s", filename, filename);
    return system(cmd);
}

int endsWith(const char *str, const char *suffix) {
    size_t len_str = strlen(str);
    size_t len_suffix = strlen(suffix);
    if (len_suffix > len_str) {
        return 0;
    }
    return strncmp(str + len_str - len_suffix, suffix, len_suffix) == 0;
}

void getRomNamesDir(const char *dir_path, const char *rom_ext, FILE *rom_names_file) {
    DIR *dir = opendir(dir_path);
    char shortname[256];
    if (dir == NULL) {
        perror("Error opening directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {  // directory
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            char sub_dir_path[1024];
            snprintf(sub_dir_path, sizeof(sub_dir_path), "%s/%s", dir_path, entry->d_name);
            getRomNamesDir(sub_dir_path, rom_ext, rom_names_file);
        } else if (entry->d_type == DT_REG) {  // regular file
            if (endsWith(entry->d_name, rom_ext)) {
                strcpy( shortname, entry->d_name);
                removeExtension(shortname);
                fprintf(rom_names_file, "%s\n", shortname);
            }
        }
    }

    closedir(dir);
}

int getRomNames(char* rom_dir_path, char* rom_names_file_path) {
    FILE *rom_names_file;
    char foldername[1256];

    sprintf( foldername, "%s%s", rom_dir_path, "/Emu");
    systems_count = findFoldersWithShortname(foldername, matching_folders, 0);
    sprintf( foldername, "%s%s", rom_dir_path, "/RApp");
    systems_count = findFoldersWithShortname(foldername, matching_folders, systems_count);

    // Open the file to write the rom names to
    rom_names_file = fopen(rom_names_file_path, "w");
    if (rom_names_file == NULL) {
        printf("Error: Failed to open file %s\n", rom_names_file_path);
        return -1;
    }
    
    for (int i = 0; i < systems_count ; i++){
        sprintf( foldername, "%s/Roms/%s" , rom_dir_path, matching_folders[i] );
        getRomNamesDir(foldername, ".zip", rom_names_file);
    }

    // Close the file and the directory
    fclose(rom_names_file);

    // Sort the rom names file
    sortFileLines(rom_names_file_path);

    return 0;
}


int matchRomNames(char* rom_names_file, char* full_rom_list_file, char* arcade_rom_names_file, char* missing_rom_names_file) {
    // Open input and output files
    FILE *rom_names_fp, *full_rom_list_fp, *arcade_rom_names_fp, *missing_rom_names_fp;
    char filename[STR_MAX];
    char* full_rom_name_first_word;

    rom_names_fp = fopen(rom_names_file, "r");
    full_rom_list_fp = fopen(full_rom_list_file, "r");
    arcade_rom_names_fp = fopen(arcade_rom_names_file, "w");
    missing_rom_names_fp = fopen(missing_rom_names_file, "w");

    if (rom_names_fp == NULL || full_rom_list_fp == NULL || arcade_rom_names_fp == NULL || missing_rom_names_fp == NULL) {
        printf("Error opening files\n");
        return 1;
    }

    // Variables for reading lines
    char rom_name[50], full_rom_name[200];

    // Read first lines from input files
    fgets(rom_name, 50, rom_names_fp);
    fgets(full_rom_name, 200, full_rom_list_fp);

    // Loop through files, writing matches to output file
    while (!feof(rom_names_fp) && !feof(full_rom_list_fp)) {
        // Strip newline characters from input strings
        strtok(rom_name, "\n");
        strtok(full_rom_name, "\n");

        // Get first word from full rom name
        strcpy(filename, full_rom_name);//preserve the original line;
        full_rom_name_first_word = strtok(filename, "\t ");        

        if (strcmp(full_rom_name_first_word, rom_name) == 0) {
            // Write matched line to output file
            fprintf(arcade_rom_names_fp, "%s\n", full_rom_name);
            // Read next line from input files
            fgets(rom_name, 50, rom_names_fp);
            fgets(full_rom_name, 200, full_rom_list_fp);
        } else if (strcmp(full_rom_name_first_word, rom_name) > 0) {
            // Skip ahead in full_rom_list_file until a match is found or EOF is reached
            while (strcmp(full_rom_name_first_word, rom_name) > 0 && !feof(rom_names_fp)) {
                fprintf(missing_rom_names_fp, "%s\n", rom_name);
                fgets(rom_name, 50, rom_names_fp);
                strtok(rom_name, "\n");
            }
        } else if (strcmp(full_rom_name_first_word, rom_name) < 0) {
            // Skip ahead in rom_names_file until a match is found or EOF is reached
            while (strcmp(full_rom_name_first_word, rom_name) < 0 && !feof(full_rom_list_fp)) {
                fgets(full_rom_name, 200, full_rom_list_fp);
                strtok(full_rom_name, "\n");
                strcpy(filename, full_rom_name);//preserve the original line;
                full_rom_name_first_word = strtok(filename, "\t ");                        
            }
        }
    }

    // Close input and output files
    fclose(rom_names_fp);
    fclose(full_rom_list_fp);
    fclose(arcade_rom_names_fp);
    fclose(missing_rom_names_fp);

    return 0;
}

int createCopyFile(const char* src_path, const char* dst_path) {
    // Check if the source file exists
    if (access(src_path, F_OK) != 0) {
        printf("The input file to copy does not exist\n");
        return -1;
    }

    // Check if the destination file exists
    if (access(dst_path, F_OK) == 0) {
        // Destination file already exists
        return 0;
    }

    // Create the destination file as a copy of the source file
    char command[1024];
    snprintf(command, sizeof(command), "cp '%s' '%s'", src_path, dst_path);
    int result = system(command);

    if (result != 0) {
        // An error occurred while creating the copy
        return -1;
    }

    // Success
    return 1;
}


void splitString(char* input, char* token1, char* token2) {
    int i, j = 0;
    int tab_pos = -1;
    int len = strlen(input);

    // Find the position of the tab character
    for (i = 0; i < len; i++) {
        if (isspace(input[i])) {
            tab_pos = i;
            break;
        }
    }

    // Extract the first token
    if (tab_pos >= 0) {
        for (i = 0; i < tab_pos; i++) {
            token1[i] = input[i];
        }
        token1[i] = '\0';

        while (isspace(input[tab_pos]))
            tab_pos++;

        // Extract the second token
        j = 0;
        if (input[tab_pos] == '\"') {
            // If the second token starts with a quote, skip it
            i = tab_pos + 1;
            while (input[i] != '\"' && i < len) {
                token2[j] = input[i];
                j++;
                i++;
            }
            // Skip the ending quote
            tab_pos = i + 1;
        } else {
            // If there are no quotes, copy the whole token
            i = tab_pos;
            while (i < len) {
                token2[j] = input[i];
                j++;
                i++;
            }
        }

        // Remove any trailing whitespace or quotes
        while (j > 0 && (isspace(token2[j - 1]) || token2[j - 1] == '\"')) {
            j--;
        }
        token2[j] = '\0';
    }
}

// Define the callback function to be used by sqlite3_exec

int updateCallback(void *data, int argc, char **argv, char **col_name)
{
    char update_sql[STR_MAX*2];
    UpdateData *d = (UpdateData *) data;

    sqlite3 *db = d->db;
    char *table_name = d->table_name;

    // Retrieve the values from the current row of the result set
    int id = atoi(argv[0]); // Assuming the first column is an integer ID
    char * path = argv[1] ; // The new value to be updated
    char *romname = basename(path);
    removeExtension(romname);
    char *title = GetGameName_func( "wathever", romname);
    if ( title != NULL ){
        // Build and execute the update statement
        sprintf(update_sql, "UPDATE %s SET disp = ? WHERE id = ?", table_name);
        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(db, update_sql, -1, &stmt, NULL);  
        sqlite3_bind_text(stmt, 1, title, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, id);
        
        rc = sqlite3_step(stmt);
        if ( rc != SQLITE_DONE) {
            fprintf(stderr, "Update failed: %s (%d)\n", sqlite3_errmsg(db), rc);
            return 1;
        }
        sqlite3_finalize(stmt);
    }
    return 0;
}

int updateSqlliteCache(char* base_dir_path) {

    char table_name[STR_MAX+6];
    char select_sql[STR_MAX*2];
    sqlite3 *db;
    UpdateData data;

    //for every system open the DB and update every occurrence
    for (int i = 0; i < systems_count ; i++){
        char cache_path[STR_MAX*3];
        //a bit of assumption here on the path, to be perfected
        snprintf(cache_path, STR_MAX*3 -1, "%s/Roms/%s/%s_cache6.db", base_dir_path, matching_folders[i], matching_folders[i]);

        if (!is_file(cache_path))
            continue; //skip this db, the update cache not found

        
        sprintf(table_name, "%s_roms", matching_folders[i]);
        int rc =  sqlite3_open(cache_path, &db);
        if ( rc != SQLITE_OK) {
            fprintf(stderr, "Cannot open database: %s (%s)\n", sqlite3_errmsg(db),
                    cache_path);
            sqlite3_close(db);
            continue;
        }
    
        sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL);

        // Build and execute the select statement to retrieve all rows from the table to be updated. 
        // we use 'path' because we are going to override 'dist', this way the tool can be run multiple times.
        data.table_name = table_name;
        data.db = db;
        sprintf(select_sql, "SELECT ID, PATH FROM %s WHERE type = 0", table_name);
        if (sqlite3_exec(db, select_sql, updateCallback, &data, NULL) != SQLITE_OK) {
            printf("Error selecting rows: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            continue;
        }

        sqlite3_exec(db, "COMMIT;", 0, 0, 0);
        // end the transaction
        sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
        sqlite3_close(db);

    }
    return 0;
}

int main(int argc, char *argv[]) {
    char* base_dir_path;
    char* list_dir_path;
    char full_rom_list_path[256];
    char arcade_rom_names_path[256];
    char missing_rom_names_path[256];
    char rom_names_file_path[256];


    if (argc < 2) {
        printf("Usage: gameNameList base_dir_path list_dir_path\n");
        return 1;
    }

    base_dir_path = argv[1];
    list_dir_path = argv[2];
    sprintf(full_rom_list_path ,"%s/%s", list_dir_path, FULL_ROM_LIST_NAME);
    sprintf(arcade_rom_names_path ,"%s/%s", list_dir_path, ARCADE_ROM_NAMES_NAME);
    sprintf(missing_rom_names_path ,"%s/%s", list_dir_path, MISSING_ROM_NAMES_NAME);
    sprintf(rom_names_file_path ,"%s/%s", list_dir_path, ROM_NAMES_FILE_NAME);


    if ( createCopyFile(arcade_rom_names_path, full_rom_list_path) < 0 ){
        return -1;
    }

    // Get the list of rom names and write them to a sorted file
    if (getRomNames(base_dir_path, rom_names_file_path) != 0) {
        printf("Error: Failed to get rom names\n");
        return -1;
    }


    // Match the rom names and write the results to the output files
    if (matchRomNames(rom_names_file_path, full_rom_list_path, arcade_rom_names_path, missing_rom_names_path) != 0) {
        printf("Error: Failed to match rom names\n");
        return -1;
    }

    //open the shared library to get the rom title from the rom short name
    char libpath[256];
    sprintf(libpath, "%s/miyoo/lib/libgamename.so", base_dir_path);
    //sprintf(libpath, "../libgamename/libgamename.so"); for debug
    void *handle = dlopen(libpath, RTLD_LAZY);
    if (handle == NULL) {
        fprintf(stderr, "Error loading library: %s\n", dlerror());
        return 1;
    }

    GetGameName_func = dlsym(handle, "GetGameNameForC");
    if (GetGameName_func == NULL) {
        fprintf(stderr, "Error retrieving symbol: %s\n", dlerror());
        dlclose(handle);
        return 1;
    }

    // Update the sql lit cache on the new colum "lab" to let the UI sort by the title name displayed instead of of the rom name
    if (updateSqlliteCache(base_dir_path) != 0 ){
        printf("Error: Failed to update sql lite caches\n");
        dlclose(handle);
        return -1;
    }

    remove(rom_names_file_path);

    dlclose(handle);

    return 0;
}
