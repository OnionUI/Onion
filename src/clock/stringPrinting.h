#ifndef STRING_PRINTING_H__
#define STRING_PRINTING_H__

typedef struct SDL_Surface SDL_Surface;
enum Color { WHITE_COLOR,
             DARK_GRAY_COLOR };

void PrintString(const char *string, SDL_Surface *sdl_screen,
                 enum Color fg_color, int x, int y);
void PrintWhiteString(const char *string, SDL_Surface *sdl_screen, int x,
                      int y);

#endif // STRING_PRINTING_H__