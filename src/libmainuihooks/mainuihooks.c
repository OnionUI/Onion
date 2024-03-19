#define _GNU_SOURCE
#include <SDL/SDL.h>
#include <SDL/SDL_rotozoom.h>
#include <SDL/SDL_ttf.h>
#include <dlfcn.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

#include "theme/config.h"
#include "theme/theme.h"
#include "utils/config.h"

#define LOGGING_FILE "/mnt/SDCARD/.tmp_update/config/.logging"
#define GET_TIME_SCRIPT "/mnt/SDCARD/.tmp_update/script/get_time.sh"
#define DEFAULT_PRELOAD "/mnt/SDCARD/miyoo/lib/libpadsp.so"

bool mainUI_initialized = false;
bool logging = false;
char theme_path[STR_MAX];
Theme_s *theme(void);

typedef struct {
    bool is_enabled;
    bool is_12h;
    SDL_Rect *position;
    TTF_Font *font;
    Time_s theme;
} TimeSettings;

TimeSettings time_settings = {false, false, NULL, NULL};
SDL_Surface *clock_surface = NULL;

typedef int (*SDL_Flip_ptr)(SDL_Surface *screen);
SDL_Flip_ptr original_SDL_Flip = NULL;
typedef int (*printf_ptr)(const char *format, ...);
printf_ptr original_printf = NULL;

char *getTime()
{
    FILE *fp;
    char output[10];
    char command[strlen(GET_TIME_SCRIPT) + 10];
    snprintf(command, sizeof(command), time_settings.is_12h ? "%s --12h" : "%s", GET_TIME_SCRIPT);
    fp = popen(command, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to run get_time.sh\n");
        return NULL;
    }

    if (fgets(output, sizeof(output), fp) == NULL) {
        fprintf(stderr, "Failed to read get_time.sh output\n");
        pclose(fp);
        return NULL;
    }

    pclose(fp);

    char *newline = strchr(output, '\n');
    if (newline != NULL)
        *newline = '\0';

    return strdup(output);
}

//
// rotate a 32bpp surface's pixels by 180 degrees without allocating new memory
//
void RotateSurface(SDL_Surface *surface)
{
    if (!surface || surface->format->BytesPerPixel != 4)
        return;

    int width = surface->w;
    int height = surface->h;
    uint32_t *pixels = (uint32_t *)surface->pixels;

    for (int y = 0; y < height / 2; y++) {
        for (int x = 0; x < width; x++) {
            uint32_t topPixel = pixels[y * width + x];
            uint32_t bottomPixel = pixels[(height - y - 1) * width + (width - x - 1)];
            pixels[y * width + x] = bottomPixel;
            pixels[(height - y - 1) * width + (width - x - 1)] = topPixel;
        }
    }

    // If the height is odd, handle the middle row separately
    if (height % 2 != 0) {
        int midY = height / 2;
        for (int x = 0; x < width / 2; x++) {
            uint32_t leftPixel = pixels[midY * width + x];
            uint32_t rightPixel = pixels[midY * width + (width - x - 1)];
            pixels[midY * width + x] = rightPixel;
            pixels[midY * width + (width - x - 1)] = leftPixel;
        }
    }
}

int printf(const char *format, ...)
{

    if (!mainUI_initialized && strcmp(format, "%d run windows\n") == 0)
        mainUI_initialized = true;

    if (!logging)
        return 0; // don't waste time printing stuff no one will see

    va_list args;
    va_start(args, format);
    int res = vprintf(format, args);
    va_end(args);

    return res;
}

int SDL_Flip(SDL_Surface *screen)
{
    if (!original_SDL_Flip) // we fucked up
        return -1;
    else if (!time_settings.is_enabled || time_settings.theme.size == 0 || !mainUI_initialized || !time_settings.font)
        return original_SDL_Flip(screen);

    char *time_string = getTime();
    clock_surface = TTF_RenderUTF8_Blended(time_settings.font, time_string, time_settings.theme.color);
    free(time_string);
    RotateSurface(clock_surface);
    SDL_BlitSurface(clock_surface, NULL, screen, time_settings.position);
    return original_SDL_Flip(screen);
}

void __attribute__((constructor)) load(void)
{
    puts(":: Hi from mainuihooks");

    // remove this .so from preload for child processes
    setenv("LD_PRELOAD", DEFAULT_PRELOAD, 1);

    // get pointers to original functions
    original_printf = dlsym(RTLD_NEXT, "printf");
    original_SDL_Flip = dlsym(RTLD_NEXT, "SDL_Flip");

    time_settings.is_12h = config_flag_get(".mainuiTime12h");
    time_settings.is_enabled = config_flag_get(".mainuiTimeEnabled");

    logging = config_flag_get(".logging");

    // load time overlay theme settings
    theme_getPath(theme_path);
    TTF_Init();
    time_settings.theme = theme()->time;
    SDL_Rect *position = calloc(1, sizeof(SDL_Rect));
    position->x = time_settings.theme.X;
    position->y = time_settings.theme.Y;
    time_settings.position = position;
    time_settings.font = theme_loadFont(theme_path, time_settings.theme.font, time_settings.theme.size);
}

void __attribute__((destructor)) unload(void)
{
    if (clock_surface)
        SDL_FreeSurface(clock_surface);
    if (time_settings.position)
        free(time_settings.position);
    TTF_Quit();

    // restore the logo
    puts(":: Bye from mainuihooks");
}