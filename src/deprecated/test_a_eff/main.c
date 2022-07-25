

#include <stdio.h>

//	libshmvar header
typedef enum {
	MONITOR_VOLUME,			// vol
	MONITOR_BRIGHTNESS,		// brightness
	MONITOR_KEYMAP,			// keymap (maybe unused)
	MONITOR_MUTE,			// mute
	MONITOR_VOLUME_CHANGED,		// volume change (internal use)
	MONITOR_BGM_VOLUME,		// bgmvol
	MONITOR_HIBERNATE_DELAY,	// hibernate
	MONITOR_ADC_VALUE,		// charging state (internal use)
	MONITOR_LUMINATION,		// lumination
	MONITOR_HUE,			// hue
	MONITOR_SATURATION,		// saturation
	MONITOR_CONTRAST,		// contrast
	MONITOR_VALUE_MAX,
} MonitorValue;

typedef struct _KeyShmInfo {
	int id;
	void *addr;
} KeyShmInfo;

int	InitKeyShm(KeyShmInfo *);
int	SetKeyShm(KeyShmInfo* info, MonitorValue key, int value);
int	GetKeyShm(KeyShmInfo* info, MonitorValue key);
int	UninitKeyShm(KeyShmInfo *);

//
//	handling libshmvar sample
//
int main() {
	KeyShmInfo	info;

	InitKeyShm(&info);

	printf("VOLUME: %d\n",		GetKeyShm(&info, MONITOR_VOLUME) );
	printf("BRIGHTNESS: %d\n",	GetKeyShm(&info, MONITOR_BRIGHTNESS) );
	printf("KEYMAP: %d\n",		GetKeyShm(&info, MONITOR_KEYMAP) );
	printf("MUTE: %d\n",		GetKeyShm(&info, MONITOR_MUTE) );
	printf("VOLUME_CHANGED: %d\n",	GetKeyShm(&info, MONITOR_VOLUME_CHANGED) );
	printf("BGM_VOLUME: %d\n",	GetKeyShm(&info, MONITOR_BGM_VOLUME) );
	printf("HIBERNATE_DELAY: %d\n",	GetKeyShm(&info, MONITOR_HIBERNATE_DELAY) );
	printf("ADC_VALUE: %d\n",	GetKeyShm(&info, MONITOR_ADC_VALUE) );
	printf("LUMINATION: %d\n",	GetKeyShm(&info, MONITOR_LUMINATION) );
	printf("HUE: %d\n",		GetKeyShm(&info, MONITOR_HUE) );
	printf("SATURATION: %d\n",	GetKeyShm(&info, MONITOR_SATURATION) );
	printf("CONTRAST: %d\n",	GetKeyShm(&info, MONITOR_CONTRAST) );

	UninitKeyShm(&info);

	return 0;
}

/*

Hello Miyoo, it is me, Christophe Jeannette alias Sichroteph/Totofaki !

*/
