#define _GNU_SOURCE
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <dlfcn.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>


typedef SDL_Surface *(*IMG_Load_t)(const char *filename);
static IMG_Load_t original_IMG_Load = NULL;

static bool is_details = false;

int printf(const char *format, ...)
{
    /* these two get printed when entering the details view                            
       "Load thumbnail" when pressing right on the rom list                            
       "GameDetailWindow::resetDetailsInternal" when pressing up from the details view */
    if (strstr(format, "Load thumbnail") != NULL ||
        strstr(format, "GameDetailWindow::resetDetailsInternal") != NULL)
        is_details = true;

    va_list args;
    va_start(args, format);
    int res = vprintf(format, args);
    va_end(args);
    return res;
}

SDL_Surface *IMG_Load(const char *filename)
{
    if (!original_IMG_Load)
        original_IMG_Load = (IMG_Load_t)dlsym(RTLD_NEXT, "IMG_Load");

    //only load "info-" thumbnails if we're in details view
    if (is_details) {
        const char *lastSlash = strrchr(filename, '/');
        char *modifiedPath = (char *)malloc(strlen(filename) + 6);
        if (modifiedPath) {

            // build "info-" path
            size_t prefixLen = lastSlash - filename + 1;
            snprintf(modifiedPath, prefixLen + 6, "%.*sinfo-", (int)prefixLen, filename);
            strcat(modifiedPath, lastSlash + 1);

            // only load the "info-" thumbnail if it exists, otherwise load the normal one
            if (access(modifiedPath, F_OK) == 0) {
                printf("[mainuihooks] loading info thumbnail %s\n", modifiedPath);
                SDL_Surface *result = original_IMG_Load(modifiedPath);
                is_details = false;
                return result;
            }
            free(modifiedPath);
        }
        is_details = false;
    }
    return original_IMG_Load(filename);
}