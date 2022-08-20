#ifndef IMAGES_CACHE_H__
#define IMAGES_CACHE_H__

struct SDL_Surface;

char* drawImageByIndex(const int index, const int image_index, char **images_paths,
    const int images_paths_count, SDL_Surface *screen, bool *cache_used);
void cleanImagesCache();

#endif // IMAGES_CACHE_H__