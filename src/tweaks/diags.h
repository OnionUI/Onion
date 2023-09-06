#ifndef TWEAKS_DIAGS_H__
#define TWEAKS_DIAGS_H__

#include <sys/wait.h>

#define DIAG_SCRIPT_PATH "/mnt/SDCARD/.tmp_update/script/diagnostics"
#define DIAG_MAX_LABEL_LENGTH 64
#define DIAG_MAX_FILENAME_LENGTH 64
#define DIAG_MAX_PATH_LENGTH (strlen(DIAG_SCRIPT_PATH) + DIAG_MAX_FILENAME_LENGTH + 2)

typedef struct {
    char filename[DIAG_MAX_FILENAME_LENGTH]; // store the filename/path so we can call it later as the payload
    char label[DIAG_MAX_LABEL_LENGTH];
    char tooltip[STR_MAX];
} diagScripts;

static int diags_numScripts;
static diagScripts *scripts = NULL;

void diags_getEntries(void)
{
    DIR *dir;
    struct dirent *ent;
    diags_numScripts = 0;

    if ((dir = opendir(DIAG_SCRIPT_PATH)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            size_t path_size = strlen(DIAG_SCRIPT_PATH) + strlen(ent->d_name) + 2;
            char *path = (char *)malloc(path_size);
            if (path == NULL) {
                printf("Memory allocation failed...\n");
                return;
            }

            snprintf(path, path_size, "%s/%s", DIAG_SCRIPT_PATH, ent->d_name);

            if (ent->d_type != DT_REG) {
                free(path);
                continue;
            }

            FILE *file = fopen(path, "r");
            if (file != NULL) {
                char line[STR_MAX];
                diagScripts entry = {0};
                strncpy(entry.filename, ent->d_name, sizeof(entry.filename) - 1);
                entry.filename[DIAG_MAX_FILENAME_LENGTH - 1] = '\0';

                while (fgets(line, sizeof(line), file)) {
                    line[strcspn(line, "\n")] = 0;
                    if (strcmp(line, "# IGNORE") == 0) {
                        break;
                    }
                    if (sscanf(line, "menulabel=\"%64[^\"]\"", entry.label) == 1) {
                        entry.label[DIAG_MAX_LABEL_LENGTH - 1] = '\0';
                        continue;
                    }
                    if (sscanf(line, "tooltip=\"%256[^\"]\"", entry.tooltip) == 1) {
                        entry.tooltip[STR_MAX - 1] = '\0';
                        continue;
                    }
                }

                fclose(file);

                if (entry.label[0] && entry.tooltip[0]) {
                    diags_numScripts++;
                    scripts = realloc(scripts, diags_numScripts * sizeof(diagScripts));
                    if (scripts == NULL) {
                        printf("Memory allocation failed...\n");
                        return;
                    }
                    scripts[diags_numScripts - 1] = entry;
                }
            }

            free(path);
        }
        closedir(dir);
    }
    else {
        printf("Could not open directory\n");
    }
}

void diags_freeEntries(void)
{
    if (scripts != NULL) {
        free(scripts);
        scripts = NULL;
    }
}

char *diags_parseNewLines(const char *input)
{ // helper function to parse /n in the scripts
    char *output = malloc(strlen(input) + 1);
    if (!output)
        return NULL;

    int j = 0;
    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] == '\\' && input[i + 1] == 'n') {
            output[j++] = '\n';
            i++;
        }
        else {
            output[j++] = input[i];
        }
    }
    output[j] = '\0';
    return output;
}

void *diags_resetStickyNote(void *payload_ptr)
{
    ListItem *item = (ListItem *)payload_ptr;

    sleep(5);

    list_updateStickyNote(item, "Idle: Selected script not running");
    list_changed = true;
    return NULL;
}

void diags_createStickyResetThread(ListItem *item)
{
    pthread_t reset_thread;
    if (pthread_create(&reset_thread, NULL, diags_resetStickyNote, item) != 0) {
        perror("Failed to create reset_thread");
    }
}

volatile int isScriptRunning = 0;

void *diags_runScript(void *payload_ptr)
{
    ListItem *item = (ListItem *)payload_ptr;
    char *filename = (char *)item->payload_ptr;

    char script_path[DIAG_MAX_PATH_LENGTH + 1];
    snprintf(script_path, sizeof(script_path), "%s/%s", DIAG_SCRIPT_PATH, filename);

    const char *currentStickyNote = list_getStickyNote(item);

    if (__sync_lock_test_and_set(&isScriptRunning, 1)) {
        if (strcmp(currentStickyNote, "Script running...") == 0) {
            list_updateStickyNote(item, "Script already running...");
        }
        else if (strcmp(currentStickyNote, "Script already running...") != 0) {
            list_updateStickyNote(item, "Another script is already running...");
            diags_createStickyResetThread(item);
        }
        list_changed = true;
        return NULL;
    }

    list_updateStickyNote(item, "Script running...");
    list_changed = true;

    pid_t pid = fork();
    if (pid == 0) {
        execl("/bin/sh", "sh", "-c", script_path, (char *)NULL);
        exit(EXIT_FAILURE);
    }
    else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            list_updateStickyNote(item, "Script successfully completed.");
        }
        else {
            list_updateStickyNote(item, "Script failed!");
        }

        diags_createStickyResetThread(item);

        __sync_lock_release(&isScriptRunning);
        list_changed = true;
    }
    else {
        list_updateStickyNote(item, "Failed to run script...");
        diags_createStickyResetThread(item);
        list_changed = true;
    }

    return NULL;
}

#endif
