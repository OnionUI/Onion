#ifndef THEME_SOUND_H__
#define THEME_SOUND_H__

#include <math.h>

#include "./resources.h"
#include "system/settings.h"
#include "utils/log.h"
#include "utils/sdl_init.h"

#ifdef HAS_AUDIO
static Mix_Chunk *_sound_change = NULL;
static bool _volume_updated = false;
#endif

void sound_change(void)
{
#ifdef HAS_AUDIO
    if (!_volume_updated) {
        int volume = (settings.bgm_volume * 128) / 20;
        printf_debug("Volume set: %d = %d\n", settings.bgm_volume, volume);
        Mix_Volume(-1, volume);
        _volume_updated = true;
    }

    if (_sound_change == NULL)
        _sound_change = resource_getSoundChange();

    Mix_PlayChannel(-1, _sound_change, 0);
#endif
}

#endif // THEME_SOUND_H__
