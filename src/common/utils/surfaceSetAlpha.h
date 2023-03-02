#ifndef UTILS_SURFACE_SET_ALPHA_H__
#define UTILS_SURFACE_SET_ALPHA_H__

#include <stdint.h>
#include <SDL/SDL.h>

// Changes a surface's alpha value, by altering per-pixel alpha if necessary.
void surfaceSetAlpha(SDL_Surface *surface, Uint8 alpha)
{
    SDL_PixelFormat* fmt = surface->format;

    // If surface has no alpha channel, just set the surface alpha.
    if( fmt->Amask == 0 ) {
        SDL_SetAlpha( surface, SDL_SRCALPHA, alpha );
    }
    // Else change the alpha of each pixel.
    else {
        unsigned bpp = fmt->BytesPerPixel;
        // Scaling factor to clamp alpha to [0, alpha].
        float scale = alpha / 255.0f;

        SDL_LockSurface(surface);

        for (int y = 0; y < surface->h; ++y) 
        for (int x = 0; x < surface->w; ++x) {
            // Get a pointer to the current pixel.
            Uint32* pixel_ptr = (Uint32 *)( 
                    (Uint8 *)surface->pixels
                    + y * surface->pitch
                    + x * bpp
                    );

            // Get the old pixel components.
            Uint8 r, g, b, a;
            SDL_GetRGBA( *pixel_ptr, fmt, &r, &g, &b, &a );

            // Set the pixel with the new alpha.
            *pixel_ptr = SDL_MapRGBA( fmt, r, g, b, scale * a );
        }   

        SDL_UnlockSurface(surface);
    }       
}

#endif // UTILS_SURFACE_SET_ALPHA_H__
