#include "imagesBrowser.h"

#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>

#define STR_MAX 256

static const char *getFilenameExt(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

static char* toLower(char* s) {
  for(char *p=s; *p; p++) *p=tolower(*p);
  return s;
}

bool loadImagesPathsFromDir(const char* dir_path, char ***images_paths, int *images_paths_count)
{
    DIR* dir;
    struct dirent *ent;
    struct stat states;

    dir = opendir(dir_path);

    if (dir == NULL) {
        return false;
    }

    *images_paths_count = 0;
    // TODO: get rid of hard-coded 100
    *images_paths = (char**)malloc(100 * sizeof(char*));

    while((ent = readdir(dir)) != NULL){
        const char *filename = ent->d_name;
        stat(filename, &states);
        if(!strcmp(".", filename) || !strcmp("..", filename)){
            continue;
        }
        else if(!S_ISDIR(ent->d_type & DT_DIR)){
            printf("%s/%s\n", dir_path, filename);
            const char *fileExt = toLower(getFilenameExt(filename));
            if (strcmp(fileExt, "png") == 0 ||
                strcmp(fileExt, "jpg") == 0 ||
                strcmp(fileExt, "jpeg") == 0) {
                printf("found image: %s\n", filename);

                (*images_paths)[*images_paths_count] = (char*)malloc(STR_MAX * sizeof(char));
                char full_path[STR_MAX];
                sprintf(full_path, "%s/%s", dir_path, filename);
                strcpy((*images_paths)[*images_paths_count], full_path);
                (*images_paths_count)++;
            }
        }
    }

    closedir(dir);

    return true;
}