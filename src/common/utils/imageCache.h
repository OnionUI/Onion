#ifndef UTILS_IMAGE_CACHE_H__
#define UTILS_IMAGE_CACHE_H__

typedef struct SDL_Surface SDL_Surface;

void imageCache_load(int *offset, SDL_Surface* (*_load_image)(int), int total);
void imageCache_removeItem(int image_index);
SDL_Surface* imageCache_getItem(int *index);
bool imageCache_isActive(void);
void imageCache_freeAll(void);

#endif // UTILS_IMAGE_CACHE_H__
