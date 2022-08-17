#include <stdbool.h>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

static SDL_Surface *g_image_cache_prev = NULL;
static SDL_Surface *g_image_cache_current = NULL;
static SDL_Surface *g_image_cache_next = NULL;

char* drawImageByIndex(const int new_image_index, const int image_index, char **images_paths,
    const int images_paths_count, SDL_Surface *screen, bool *cache_used)
{
	if (new_image_index < 0 || new_image_index >= images_paths_count)
	{
		// out of range, draw nothing
		printf("out of range, draw nothing\n");
		return NULL;
	}
	char* image_path_to_draw = images_paths[new_image_index];
	if (new_image_index == image_index)
	{
		if (g_image_cache_current == NULL)
		{
			printf("invalidating cache\n");
			g_image_cache_prev = new_image_index == 0 ? NULL : IMG_Load(images_paths[new_image_index - 1]);
			g_image_cache_current = IMG_Load(images_paths[new_image_index]);
			g_image_cache_next = new_image_index == images_paths_count - 1 ? NULL : IMG_Load(images_paths[new_image_index + 1]);
			*cache_used = false;
			return image_path_to_draw;
		}
		
		printf("no movements, draw nothing\n");
		return NULL;
	}
	if (abs(new_image_index - image_index) > 1)
	{
		printf("random jump, not implemented yet\n");
		return NULL;
	}

	bool moving_forward = new_image_index > image_index;

	if (moving_forward)
	{
		printf("moving forward\n");
		if (g_image_cache_prev)
			SDL_FreeSurface(g_image_cache_prev);
		g_image_cache_prev = g_image_cache_current;
		g_image_cache_current = g_image_cache_next;
		g_image_cache_next = new_image_index == images_paths_count - 1 ? NULL : IMG_Load(images_paths[new_image_index+1]);
		*cache_used = true;
	}
	else
	{
		printf("moving backward\n");
		g_image_cache_prev = new_image_index == 0 ? NULL : IMG_Load(images_paths[new_image_index-1]);
		if (g_image_cache_next)
			SDL_FreeSurface(g_image_cache_next);
		g_image_cache_next = g_image_cache_current;
		g_image_cache_current = g_image_cache_prev;
		*cache_used = true;
	}
	SDL_Surface *image_to_draw = g_image_cache_current;
	if (image_to_draw) {
		SDL_Rect image_rect = {(int16_t)(320 - image_to_draw->w / 2), (int16_t)(240 - image_to_draw->h / 2)};
		SDL_BlitSurface(image_to_draw, NULL, screen, &image_rect);
	}
	
	return image_path_to_draw;
}