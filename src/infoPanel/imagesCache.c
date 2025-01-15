#include <stdbool.h>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_rotozoom.h>

#include "utils/log.h"

static SDL_Surface *g_image_cache_prev = NULL;
static SDL_Surface *g_image_cache_current = NULL;
static SDL_Surface *g_image_cache_next = NULL;

#ifdef LOG_DEBUG
#define DEBUG_PRINT(x) printf x
#else
#define DEBUG_PRINT(x) \
    do {               \
    } while (0)
#endif

#define MIN(a, b) (a < b) ? (a) : (b)

SDL_Surface *scaleImageIfNecessary(SDL_Surface *image, SDL_Rect target, bool stretch)
{
    if (image->w > target.w || image->h > target.h || (stretch && (image->w < target.w || image->h < target.h))) {
        double ratio_x = (double)target.w / image->w;
        double ratio_y = (double)target.h / image->h;
        double scale = MIN(ratio_x, ratio_y);

        SDL_Surface *scaledImage = zoomSurface(image, scale, scale, SMOOTHING_OFF);

        if (scaledImage) {
            printf_debug("scaled image from %dx%d to %dx%d\n", image->w, image->h, scaledImage->w, scaledImage->h);
            return scaledImage;
        }
        else {
            printf("zoomSurface failed: %s\n", SDL_GetError());
        }
    }

    return NULL;
}

SDL_Surface *loadImage(const char *image_path, SDL_Surface *screen)
{
    SDL_Surface *image = IMG_Load(image_path);

    if (!image) {
        printf("Error loading image: %s\n", image_path);
        return NULL;
    }

    SDL_Surface *scaledImage = scaleImageIfNecessary(image, screen->clip_rect, false);
    if (scaledImage) {
        SDL_FreeSurface(image);
        return scaledImage;
    }

    return image;
}

SDL_Rect getCenterPos(SDL_Surface *image, SDL_Rect target)
{
    SDL_Rect image_pos = {
        target.x + (target.w - image->w) / 2,
        target.y + (target.h - image->h) / 2};

    return image_pos;
}

void drawImage(SDL_Surface *image, SDL_Surface *screen, const SDL_Rect *frame)
{
    if (!image)
        return;

    SDL_Rect target = {0, 0, screen->w, screen->h};

    if (frame != NULL) {
        target = *frame;
    }

    SDL_Surface *scaledImage = scaleImageIfNecessary(image, target, false);
    if (scaledImage) {
        SDL_Rect image_pos = getCenterPos(scaledImage, target);
        SDL_BlitSurface(scaledImage, NULL, screen, &image_pos);
        SDL_FreeSurface(scaledImage);
        return;
    }

    SDL_Rect image_pos = getCenterPos(image, target);
    SDL_BlitSurface(image, NULL, screen, &image_pos);
}

char *drawImageByIndex(const int new_image_index, const int image_index,
                       char **images_paths, const int images_paths_count,
                       SDL_Surface *screen, SDL_Rect *frame, bool *cache_used)
{
    DEBUG_PRINT(("image_index: %d, new_image_index: %d\n", image_index,
                 new_image_index));

    if (new_image_index < 0 || new_image_index >= images_paths_count) {
        // out of range, draw nothing
        printf("out of range, draw nothing\n");
        return NULL;
    }
    char *image_path_to_draw = images_paths[new_image_index];
    DEBUG_PRINT(("image_path_to_draw: %s\n", image_path_to_draw));
    if (new_image_index == image_index) {
        if (g_image_cache_current == NULL) {
            DEBUG_PRINT(("invalidating cache\n"));
            g_image_cache_prev =
                new_image_index == 0
                    ? NULL
                    : loadImage(images_paths[new_image_index - 1], screen);
            g_image_cache_current = loadImage(images_paths[new_image_index], screen);
            g_image_cache_next =
                new_image_index == images_paths_count - 1
                    ? NULL
                    : loadImage(images_paths[new_image_index + 1], screen);

            drawImage(g_image_cache_current, screen, frame);

            *cache_used = false;
            return image_path_to_draw;
        }
    }
    if (abs(new_image_index - image_index) > 1) {
        DEBUG_PRINT(("random jump, not implemented yet\n"));
        return NULL;
    }

    int move_direction = new_image_index - image_index;

    if (move_direction > 0) {
        DEBUG_PRINT(("moving forward\n"));
        if (g_image_cache_prev)
            SDL_FreeSurface(g_image_cache_prev);
        g_image_cache_prev = g_image_cache_current;
        g_image_cache_current = g_image_cache_next;
        if (new_image_index == images_paths_count - 1) {
            g_image_cache_next = NULL;
        }
        else {
            const int next_image_index = new_image_index + 1;
            char *image_path_to_load = images_paths[next_image_index];
            DEBUG_PRINT(("preloading next image '%s' for index #%d\n",
                         image_path_to_load, next_image_index));
            g_image_cache_next = loadImage(image_path_to_load, screen);
        }
        *cache_used = true;
    }
    else if (move_direction < 0) {
        DEBUG_PRINT(("moving backward\n"));

        if (g_image_cache_next)
            SDL_FreeSurface(g_image_cache_next);
        g_image_cache_next = g_image_cache_current;
        g_image_cache_current = g_image_cache_prev;
        if (new_image_index == 0) {
            g_image_cache_prev = NULL;
        }
        else {
            const int prev_image_index = new_image_index - 1;
            char *image_path_to_load = images_paths[prev_image_index];
            DEBUG_PRINT(("preloading prev image '%s' for index #%d\n",
                         image_path_to_load, prev_image_index));
            g_image_cache_prev = loadImage(image_path_to_load, screen);
        }

        *cache_used = true;
    }
    else {
        DEBUG_PRINT(("same slide\n"));
        *cache_used = true;
    }

    drawImage(g_image_cache_current, screen, frame);

    return image_path_to_draw;
}

void cleanImagesCache()
{
    DEBUG_PRINT(("cleaning images cache\n"));
    if (g_image_cache_prev)
        SDL_FreeSurface(g_image_cache_prev);
    if (g_image_cache_current)
        SDL_FreeSurface(g_image_cache_current);
    if (g_image_cache_next)
        SDL_FreeSurface(g_image_cache_next);
}
