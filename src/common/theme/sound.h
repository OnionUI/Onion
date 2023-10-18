#ifndef THEME_SOUND_H__
#define THEME_SOUND_H__

#include <math.h>

#include "./resources.h"
#include "system/settings.h"
#include "utils/log.h"
#include "utils/sdl_init.h"
#include "utils/file.h"

static Mix_Chunk *_sound_change = NULL;
static bool _volume_updated = false;

void sound_change(void)
{
    if (!sdl_has_audio)
        return;

    if (!_volume_updated || exists("/tmp/volume_changed")) {
        int volume = settings.bgm_volume > 0
                         ? 42.3936 * log(1.0239 * (double)settings.bgm_volume)
                         : 0;
        printf_debug("Volume set: %d = %d\n", settings.bgm_volume, volume);
        Mix_Volume(-1, volume);
        _volume_updated = true;
        system("rm -f /tmp/volume_changed");
    }

    if (_sound_change == NULL)
        _sound_change = resource_getSoundChange();

    Mix_PlayChannel(-1, _sound_change, 0);
}

#endif // THEME_SOUND_H__
