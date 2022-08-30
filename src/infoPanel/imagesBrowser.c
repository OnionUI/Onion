#include "imagesBrowser.h"

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <linux/limits.h>

#define STR_MAX 256

static const char *getFilenameExt(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

static char* toLower(char* s) {
  for (char *p = s; *p; p++)
  {
    *p = tolower(*p);
  }
  return s;
}

static bool getImagePath(const char *dir_path, const struct dirent *ent, char *image_path) {
    const int ext_size = 50;
    char ext[ext_size];
    const char *filename = ent->d_name;
    if (filename[0] == '.' || S_ISDIR(ent->d_type & DT_DIR))
    {
        return false;
    }
    strncpy(ext, getFilenameExt(filename), ext_size);
    const char *fileExt = toLower(ext);
    if (strcmp(fileExt, "png") == 0 || strcmp(fileExt, "jpg") == 0 || strcmp(fileExt, "jpeg") == 0)
    {
        char full_path[PATH_MAX];
        sprintf(full_path, "%s%s", dir_path, filename);
        strcpy(image_path, full_path);
        return true;
    }
    return false;

}

static int getImagesCount(const char* dir_path) {
    int images_count = 0;

    DIR* dir = opendir(dir_path);
    if (dir == NULL)
    {
        return 0;
    }

    struct dirent *ent;
    
    while((ent = readdir(dir)) != NULL)
    {
        char image_path[PATH_MAX];
        const bool is_image = getImagePath(dir_path, ent, image_path);
        if (is_image)
        {
            images_count++;
        }
    }

    closedir(dir);
    

    return images_count;
}

int compare_strings(const void* a, const void* b)
{
    const char* aa = *(const char**)a;
    const char* bb = *(const char**)b;
    return strcmp(aa,bb);
}

bool loadImagesPathsFromDir(const char* dir_path, char ***images_paths, int *images_paths_count)
{
    char normalized_dir_path[PATH_MAX];
    const int dir_path_length = strlen(dir_path);
    if (dir_path_length == 0) {
        return false;
    }
    if (dir_path[dir_path_length-1] != '/')
    {
        sprintf(normalized_dir_path, "%s/", dir_path);
    }
    else {
        strcpy(normalized_dir_path, dir_path);
    }

    const int images_count = getImagesCount(normalized_dir_path);
    
    DIR *dir = opendir(normalized_dir_path);

    if (dir == NULL) {
        return false;
    }

    struct dirent *ent;

    *images_paths_count = 0;
    *images_paths = (char**)malloc(images_count * sizeof(char*));

    while((ent = readdir(dir)) != NULL){
        char image_path[PATH_MAX];
        const bool is_image = getImagePath(normalized_dir_path, ent, image_path);

        if (!is_image) {
            continue;
        }

        (*images_paths)[*images_paths_count] = (char*)malloc(PATH_MAX * sizeof(char));
        strcpy((*images_paths)[*images_paths_count], image_path);
        (*images_paths_count)++;
        
        if ((*images_paths_count) >= images_count) {
            // we found more images than allocated memory
            // TODO: handle this
            break;
        }
    }

    closedir(dir);

    qsort(*images_paths, *images_paths_count, sizeof(char *), compare_strings);

    return true;
}