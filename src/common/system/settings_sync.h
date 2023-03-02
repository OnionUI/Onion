#ifndef SETTINGS_SYNC_H__
#define SETTINGS_SYNC_H__

#include <stdbool.h>

#ifdef PLATFORM_MIYOOMINI
#include "shmvar/shmvar.h"
static KeyShmInfo shminfo;
#endif

#include "settings.h"

void settings_sync(void)
{
    #ifdef PLATFORM_MIYOOMINI
    SetKeyShm(&shminfo, MONITOR_VOLUME, settings.volume);
    SetKeyShm(&shminfo, MONITOR_BRIGHTNESS, settings.brightness);
    SetKeyShm(&shminfo, MONITOR_KEYMAP, 0);
    SetKeyShm(&shminfo, MONITOR_MUTE, settings.mute);
    SetKeyShm(&shminfo, MONITOR_VOLUME_CHANGED, 0);
    SetKeyShm(&shminfo, MONITOR_BGM_VOLUME, settings.bgm_volume);
    SetKeyShm(&shminfo, MONITOR_HIBERNATE_DELAY, settings.sleep_timer);
    SetKeyShm(&shminfo, MONITOR_LUMINATION, settings.lumination);
    SetKeyShm(&shminfo, MONITOR_HUE, settings.hue);
    SetKeyShm(&shminfo, MONITOR_SATURATION, settings.saturation);
    SetKeyShm(&shminfo, MONITOR_CONTRAST, settings.contrast);
    SetKeyShm(&shminfo, MONITOR_AUDIOFIX, settings.audiofix);
    #endif
}

void settings_init(void)
{
    #ifdef PLATFORM_MIYOOMINI
    InitKeyShm(&shminfo);

    // Disable MainUI battery monitor
    SetKeyShm(&shminfo, MONITOR_ADC_VALUE, 640);
    #endif

    settings_load();
    settings_sync();
}


#endif // SETTINGS_SYNC_H__
