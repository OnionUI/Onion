#ifndef SETTINGS_SYNC_H__
#define SETTINGS_SYNC_H__

#include <stdbool.h>

#ifdef PLATFORM_MIYOOMINI
#include "shmvar/shmvar.h"
static KeyShmInfo shminfo;
#endif

#include "./clock.h"
#include "./settings.h"

#define VOLUME_DISPLAY_TIMEOUT 2000

static int _volume_changed = 0;
static int _changed_timer = 0;

// Currently not working, has a delay which means the last volume level is
// shown, not the current
void settings_show_volume_change()
{
    // _changed_timer = getMilliseconds();
    // _volume_changed = 1;
#ifdef PLATFORM_MIYOOMINI
    // SetKeyShm(&shminfo, MONITOR_VOLUME_CHANGED, _volume_changed);
#endif
}

void settings_shm_write(void)
{
#ifdef PLATFORM_MIYOOMINI
    SetKeyShm(&shminfo, MONITOR_BRIGHTNESS, settings.brightness);
    SetKeyShm(&shminfo, MONITOR_VOLUME, settings.volume);
    SetKeyShm(&shminfo, MONITOR_BGM_VOLUME, settings.bgm_volume);

    SetKeyShm(&shminfo, MONITOR_HIBERNATE_DELAY, settings.sleep_timer);

    SetKeyShm(&shminfo, MONITOR_LUMINATION, settings.lumination);
    SetKeyShm(&shminfo, MONITOR_HUE, settings.hue);
    SetKeyShm(&shminfo, MONITOR_SATURATION, settings.saturation);
    SetKeyShm(&shminfo, MONITOR_CONTRAST, settings.contrast);

    SetKeyShm(&shminfo, MONITOR_KEYMAP, 0);
    SetKeyShm(&shminfo, MONITOR_VOLUME_CHANGED, _volume_changed);
    SetKeyShm(&shminfo, MONITOR_AUDIOFIX, settings.audiofix);
#endif
}

bool _has_changed(int new_value, int *old_value)
{
    if (new_value != *old_value) {
        *old_value = new_value;
        return true;
    }

    return false;
}

void settings_shm_read(void)
{
    bool has_changes = false;

    if (getMilliseconds() - _changed_timer > VOLUME_DISPLAY_TIMEOUT) {
        _volume_changed = 0;
#ifdef PLATFORM_MIYOOMINI
        SetKeyShm(&shminfo, MONITOR_VOLUME_CHANGED, _volume_changed);
#endif
    }

#ifdef PLATFORM_MIYOOMINI
    if (_has_changed(GetKeyShm(&shminfo, MONITOR_BRIGHTNESS),
                     &settings.brightness))
        has_changes = true;
    // if (_has_changed(GetKeyShm(&shminfo, MONITOR_VOLUME), &settings.volume))
    //     has_changes = true;
    if (_has_changed(GetKeyShm(&shminfo, MONITOR_BGM_VOLUME),
                     &settings.bgm_volume))
        has_changes = true;
    if (_has_changed(GetKeyShm(&shminfo, MONITOR_HIBERNATE_DELAY),
                     &settings.sleep_timer))
        has_changes = true;
    if (_has_changed(GetKeyShm(&shminfo, MONITOR_LUMINATION),
                     &settings.lumination))
        has_changes = true;
    if (_has_changed(GetKeyShm(&shminfo, MONITOR_HUE), &settings.hue))
        has_changes = true;
    if (_has_changed(GetKeyShm(&shminfo, MONITOR_SATURATION),
                     &settings.saturation))
        has_changes = true;
    if (_has_changed(GetKeyShm(&shminfo, MONITOR_CONTRAST), &settings.contrast))
        has_changes = true;
#endif

    if (has_changes)
        _settings_save_mainui();
}

void settings_init(void)
{
#ifdef PLATFORM_MIYOOMINI
    InitKeyShm(&shminfo);

    // Disable MainUI battery monitor
    SetKeyShm(&shminfo, MONITOR_ADC_VALUE, 640);
#endif

    settings_load();
    settings_shm_write();
}

#endif // SETTINGS_SYNC_H__
