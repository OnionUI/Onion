#define _GNU_SOURCE
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <dlfcn.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "system/battery.h"
#include "theme/theme.h"
#include "utils/file.h"

#define PREFIX "\x1B[31m[MainUIHooks]\x1B[0m "
#define NUM_BAT_SURFACES 5
#define MAX_PATH 256
#define BATICON_UPDATE_INTERVAL 10 // seconds

typedef int (*SDL_Flip_t)(SDL_Surface *screen);
typedef int (*SDL_UpperBlit_t)(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect);
typedef SDL_Surface *(*IMG_Load_t)(const char *file);
typedef SDL_Surface *(*SDL_ConvertSurface_t)(SDL_Surface *src, SDL_PixelFormat *fmt, Uint32 flags);

static SDL_Flip_t original_SDL_Flip = NULL;
static SDL_UpperBlit_t original_SDL_UpperBlit = NULL;
static IMG_Load_t original_IMG_Load = NULL;
static SDL_ConvertSurface_t original_SDL_ConvertSurface = NULL;

static SDL_Surface *batPercSurface[NUM_BAT_SURFACES];
static SDL_Surface *convBatPercSurface[NUM_BAT_SURFACES];
static SDL_Surface *batIcon = NULL;

static char batIconPath[MAX_PATH + 16];
static char themePath[MAX_PATH];
int batSurfaceCount = 0;
static pthread_t updateBatteryIconThread;
static pthread_mutex_t batIconMutex = PTHREAD_MUTEX_INITIALIZER;

// updates the battery percentage icon every BATICON_UPDATE_INTERVAL seconds
static void *updateBatteryPercentage()
{
    while (true) {
        while (batIconPath[0] == '\0')
            usleep(100000); // wait for batIconPath to be set in IMG_Load

        TTF_Init();
        int percentage = battery_getPercentage();

        pthread_mutex_lock(&batIconMutex);
        if (batIcon != NULL)
            SDL_FreeSurface(batIcon);
        batIcon = theme_batterySurfaceWithBg(percentage, theme_background());
        pthread_mutex_unlock(&batIconMutex);

        TTF_Quit();
        sleep(BATICON_UPDATE_INTERVAL);
    }
    return NULL;
}

/* ################################################# */
/* #  The following functions are hooked functions # */
/* ################################################# */

SDL_Surface *IMG_Load(const char *file)
{
    if (strstr(file, ".batt-perc.png") != NULL) {
        // collect and save the 5 battery percentage surfaces
        // these are converted and freed by MainUI immediately after being loaded
        // we only need them to find the correct converted surfaces

        printf(PREFIX "IMG_Load batt-perc.png\n");
        if (batIconPath[0] == '\0') {
            strcpy(themePath, extractPath(file));
            sprintf(batIconPath, "%s.batt-perc.png", themePath);
            printf(PREFIX "batIconPath: %s\n", batIconPath);

            pthread_mutex_lock(&batIconMutex);
            batIcon = original_IMG_Load(batIconPath);
            pthread_mutex_unlock(&batIconMutex);
        }
        batPercSurface[batSurfaceCount] = original_IMG_Load(file);

        return batPercSurface[batSurfaceCount];
    }
    return original_IMG_Load(file);
}

SDL_Surface *SDL_ConvertSurface(SDL_Surface *src, SDL_PixelFormat *fmt, Uint32 flags)
{
    if (src == batPercSurface[batSurfaceCount]) {
        // convert and save the 5 battery percentage surfaces
        // these are the ones that MainUI will blit

        printf(PREFIX "SDL_ConvertSurface bat-perc.png\n");
        convBatPercSurface[batSurfaceCount] = original_SDL_ConvertSurface(src, fmt, flags);
        return convBatPercSurface[batSurfaceCount++];
    }
    return original_SDL_ConvertSurface(src, fmt, flags);
}

int SDL_UpperBlit(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect)
{
    // if we are blitting one of the 5 battery percentage surfaces, blit ours instead
    // MainUI blits the battery icon at least once a second

    for (int i = 0; i < NUM_BAT_SURFACES; i++)
        if (convBatPercSurface[i] == src) {

            pthread_mutex_lock(&batIconMutex);
            int result;
            if (batIcon != NULL)
                result = original_SDL_UpperBlit(batIcon, srcrect, dst, dstrect);
            else
                result = 0;
            pthread_mutex_unlock(&batIconMutex);

            return result;
        }
    // otherwise, blit normally
    return original_SDL_UpperBlit(src, srcrect, dst, dstrect);
}

int SDL_Flip(SDL_Surface *screen)
{
    // leave this for future use
    // here we can add arbitrary overlays over MainUI

    return original_SDL_Flip(screen);
}

/* ################################################# */
/* #  Constructor and destructor for this library  # */
/* ################################################# */

void __attribute__((constructor)) libmainuihooks(void)
{
    // Remove this lib from LD_PRELOAD, it messes with subprocesses
    setenv("LD_PRELOAD", "/mnt/SDCARD/miyoo/lib/libpadsp.so", 1);

    printf(PREFIX "Hi from libmainuihooks\n");

    // get references to the original functions
    original_SDL_Flip = (SDL_Flip_t)dlsym(RTLD_NEXT, "SDL_Flip");
    original_SDL_UpperBlit = (SDL_UpperBlit_t)dlsym(RTLD_NEXT, "SDL_UpperBlit");
    original_IMG_Load = (IMG_Load_t)dlsym(RTLD_NEXT, "IMG_Load");
    original_SDL_ConvertSurface = (SDL_ConvertSurface_t)dlsym(RTLD_NEXT, "SDL_ConvertSurface");

    // start the battery percentage update thread
    pthread_create(&updateBatteryIconThread, NULL, updateBatteryPercentage, NULL);
    pthread_detach(updateBatteryIconThread);
}

void __attribute__((destructor)) libmainuihooks_end(void)
{
    pthread_mutex_lock(&batIconMutex);
    if (batIcon != NULL)
        SDL_FreeSurface(batIcon);
    pthread_mutex_unlock(&batIconMutex);

    printf(PREFIX "Bye from libmainuihooks\n");
}
