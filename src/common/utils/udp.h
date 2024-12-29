#ifndef UTILS_UDP_H__
#define UTILS_UDP_H__

#include <arpa/inet.h>
#include <stdbool.h>

typedef struct RetroArchInfo {
    unsigned int max_disk_slots;
    unsigned int disk_slot;
    int state_slot;
    bool has_state_slot;
} RetroArchInfo_t;

int udp_send(const char *ipAddress, int port, const char *message);
int udp_send_receive(const char *ipAddress, int port, const char *message, char *response, size_t response_size);

int retroarch_cmd(const char *cmd);                                       // Send RetroArch UDP command
int retroarch_get(const char *cmd, char *response, size_t response_size); // Get RetroArch UDP command response

int retroarch_quit(void);       // RetroArch QUIT
int retroarch_toggleMenu(void); // RetroArch MENU_TOGGLE
int retroarch_getInfo(RetroArchInfo_t *info);

#endif // UTILS_UDP_H__
