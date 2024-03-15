#define _GNU_SOURCE
#include <SDL/SDL.h>
#include <SDL/SDL_rotozoom.h>
#include <SDL/SDL_ttf.h>
#include <dlfcn.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

#define LOGGING_FILE "/mnt/SDCARD/.tmp_update/config/.logging"
#define GET_TIME_SCRIPT "/mnt/SDCARD/.tmp_update/script/get_time.sh"
#define TIME_FONT_FILE "/customer/app/Exo-2-Bold-Italic.ttf"
#define DEFAULT_PRELOAD "/mnt/SDCARD/miyoo/lib/libpadsp.so"

typedef int (*SDL_Flip_ptr)(SDL_Surface *screen);
SDL_Flip_ptr original_SDL_Flip = NULL;
typedef int (*printf_ptr)(const char *format, ...);
printf_ptr original_printf = NULL;

SDL_Surface *clock_surface = NULL;
TTF_Font *font = NULL;
SDL_Color white = {255, 255, 255};

bool mainUI_initialized = false;
bool logging = false;

char *getTime()
{
    //TODO: 12/24h?
    FILE *fp;
    char output[6];

    fp = popen(GET_TIME_SCRIPT, "r");
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
// only works correctly for surfaces of even height
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
    else if (!mainUI_initialized || !font)
        return original_SDL_Flip(screen);

    char *time_string = getTime();
    clock_surface = TTF_RenderUTF8_Blended(font, time_string, white);
    free(time_string);
    RotateSurface(clock_surface);

    // TODO: coords from theme config?
    SDL_Rect dest = {160, 467 - (clock_surface->h), 0, 0};

    SDL_BlitSurface(clock_surface, NULL, screen, &dest);
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

    // check if logging is enabled
    if (access(LOGGING_FILE, F_OK) == 0)
        logging = true;

    // load font
    TTF_Init();

    // TODO: font size from theme config?
    font = TTF_OpenFont(TIME_FONT_FILE, 24);
    if (!font)
        printf("TTF_OpenFont: %s\n", TTF_GetError());
}

void __attribute__((destructor)) unload(void)
{
    if (font)
        TTF_CloseFont(font);
    if (clock_surface)
        SDL_FreeSurface(clock_surface);
    TTF_Quit();

    puts(":: Bye from mainuihooks");
}