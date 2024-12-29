#ifndef UTILS_RETROARCH_CMD_H__
#define UTILS_RETROARCH_CMD_H__

#include <stdbool.h>
#include <stddef.h>

typedef struct RetroArchInfo {
    unsigned int max_disk_slots;
    unsigned int disk_slot;
    int state_slot;
    bool has_state_slot;
} RetroArchInfo_t;

int retroarch_cmd(const char *cmd);                                       // Send RetroArch UDP command
int retroarch_get(const char *cmd, char *response, size_t response_size); // Get RetroArch UDP command response

int retroarch_quit(void);                     // RetroArch QUIT
int retroarch_toggleMenu(void);               // RetroArch MENU_TOGGLE
int retroarch_getInfo(RetroArchInfo_t *info); // Get RetroArch info

#endif // UTILS_RETROARCH_CMD_H__