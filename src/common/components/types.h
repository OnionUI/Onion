
#ifndef TYPES_H__
#define TYPES_H__

typedef struct {
    char name[STR_MAX - 11];
    char path[STR_MAX];
    int available;     // 1 if available = yes, 0 otherwise
    long availablePos; // in file position for the available property
} Share;

typedef struct {
    char bssid[18];
    int frequency;
    int signal_level;
    char flags[256];
    char ssid[33];
    bool connected;
    bool encrypted;
    bool known;
    int id;
} WifiNetwork;

#endif // TYPES_H__