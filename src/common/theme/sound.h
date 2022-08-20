#ifndef THEME_SOUND_H__
#define THEME_SOUND_H__

#include "utils/sdl_init.h"
#include "./resources.h"

static Mix_Chunk *_sound_change = NULL;

void sound_change(void)
{
    if (!sdl_has_audio)
        return;

    if (_sound_change == NULL)
        _sound_change = resource_getSoundChange();

    Mix_PlayChannel(-1, _sound_change, 0);
}

#endif // THEME_SOUND_H__
