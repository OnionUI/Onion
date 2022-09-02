#ifndef IMAGES_CACHE_H__
#define IMAGES_CACHE_H__

#ifdef __cplusplus
extern "C" {
#endif

struct SDL_Surface;

char* drawImageByIndex(const int index, const int image_index, char **images_paths,
    const int images_paths_count, SDL_Surface *screen, bool *cache_used);
void cleanImagesCache();

#ifdef __cplusplus
}
#endif

#endif // IMAGES_CACHE_H__