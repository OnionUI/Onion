#ifndef __SHM_VAR__
#define __SHM_VAR__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3) ((unsigned int)(unsigned char)(ch0) | ((unsigned int)(unsigned char)(ch1) << 8) | ((unsigned int)(unsigned char)(ch2) << 16) | ((unsigned int)(unsigned char)(ch3) << 24 ))
#endif

#define KEYMON_SHMKEYID   MAKEFOURCC('K','M','O','N')


typedef enum {
    MONITOR_VOLUME,
    MONITOR_BRIGHTNESS,
    MONITOR_KEYMAP,
    MONITOR_MUTE,
    MONITOR_VOLUME_CHANGED,
    MONITOR_BGM_VOLUME,
    MONITOR_HIBERNATE_DELAY,
    MONITOR_VALUE_MAX,
}MonitorValue;

typedef struct _KeyShmInfo{
    int id;
    void *addr;
}KeyShmInfo;


int InitKeyShm(KeyShmInfo *);
int SetKeyShm(KeyShmInfo* info, MonitorValue key, int value);
int GetKeyShm(KeyShmInfo* info, MonitorValue key);
int UninitKeyShm(KeyShmInfo *);


#ifdef __cplusplus
}
#endif

#endif