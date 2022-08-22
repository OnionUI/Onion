#include <SDL/SDL.h>
#include <pthread.h>
#include <stdbool.h>

#include "utils/log.h"
#include "imageCache.h"

static pthread_t thread_pt;
static bool thread_active = false;

static const int image_cache_len = 10;
static SDL_Surface *image_cache[10] = {NULL};
static int image_cache_offset = -1;

static SDL_Surface* (*load_image)(int) = NULL;
static int images_total = 0;

static void* _imageCacheThread(void *param)
{
    int offset = *((int*)param) - image_cache_len / 2;
    int start = image_cache_offset - image_cache_len + 1;
    int end = image_cache_offset;

    if (offset < 0)
        offset = 0;
    else if (offset > images_total - image_cache_len)
        offset = images_total - image_cache_len;

    for (int i = 0; i < image_cache_len; i++) {
        int curr = offset + i;

        if (curr >= images_total)
            break;

        if (curr >= start && curr <= end)
            continue;

        int idx = curr % image_cache_len;

        if (image_cache[idx] != NULL)
            SDL_FreeSurface(image_cache[idx]);

        if (load_image != NULL)
            image_cache[idx] = load_image(curr);
    }

    image_cache_offset = offset + image_cache_len - 1;

    thread_active = false;
    return 0;
}

void imageCache_load(int *offset, SDL_Surface* (*_load_image)(int), int total)
{
    if (thread_active)
        return;
    thread_active = true;
    load_image = _load_image;
    images_total = total;
    pthread_create(&thread_pt, NULL, _imageCacheThread, offset);
}

void imageCache_removeItem(int image_index)
{
    int start = image_cache_offset - image_cache_len + 1;
    int end = image_cache_offset;

    if (image_index < start || image_index > end)
        return;

    printf_debug("Removing image %d (%d)\n", image_index, image_index % image_cache_len);

    SDL_Surface *curr_item = image_cache[image_index % image_cache_len];
    if (curr_item != NULL)
        SDL_FreeSurface(curr_item);

    int num_images = (images_total < image_cache_len ? images_total : image_cache_len) - 1;
    for (int i = 0; i < num_images; i++) {
        int curr = (image_index + i) % image_cache_len;
        int next = (image_index + i + 1) % image_cache_len;
        image_cache[curr] = image_cache[next];
        image_cache[next] = NULL;
    }

    image_cache_offset = image_index + image_cache_len - 2;
}

SDL_Surface* imageCache_getItem(int *index)
{
    imageCache_load(index, load_image, images_total);
    int idx = *index % image_cache_len;
    return image_cache[idx];
}

bool imageCache_isActive(void)
{
    return thread_active;
}

void imageCache_freeAll(void)
{
    for (int i = 0; i < image_cache_len; i++) {
        if (image_cache[i] != NULL)
            SDL_FreeSurface(image_cache[i]);
    }
}