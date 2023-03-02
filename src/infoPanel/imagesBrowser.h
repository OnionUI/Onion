#ifndef IMAGES_BROWSER_H__
#define IMAGES_BROWSER_H__

#include <stdbool.h>

bool loadImagesPathsFromDir(const char* dir_path, char ***images_paths, int *images_paths_count);

#endif // IMAGES_BROWSER_H__