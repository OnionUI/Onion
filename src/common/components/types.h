
#ifndef TYPES_H__
#define TYPES_H__

#define SSID_MAX_LENGTH 32

typedef struct {
    char name[STR_MAX - 11];
    char path[STR_MAX];
    int available;     // 1 if available = yes, 0 otherwise
    long availablePos; // in file position for the available property
} Share;

typedef struct WifiNetwork {
    char bssid[18];
    int frequency;
    int signal_level;
    char flags[256];
    char ssid[65]; // 32 * 2 because of escape characters + null terminator
    bool connected;
    bool encrypted;
    bool known;
    int id;
    struct WifiNetwork *next;
} WifiNetwork;

typedef struct {
    int id;
    char SSID[SSID_MAX_LENGTH];
    char flags[64]; // arbitrary number
} KnownNetwork;

typedef struct { // linked list for wifi networks
    WifiNetwork *head;
    int count;
} WifiNetworkList;

#endif // TYPES_H__