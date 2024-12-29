#include "retroarch_cmd.h"

#include <stdio.h>

#include "udp.h"

static const char *LOCALHOST_IP = "127.0.0.1";
static const int RETROARCH_CMD_UDP_PORT = 55355;

int retroarch_cmd(const char *cmd)
{
    return udp_send(LOCALHOST_IP, RETROARCH_CMD_UDP_PORT, cmd);
}

int retroarch_get(const char *cmd, char *response, size_t response_size)
{
    return udp_send_receive(LOCALHOST_IP, RETROARCH_CMD_UDP_PORT, cmd, response, response_size);
}

int retroarch_quit(void)
{
    return retroarch_cmd("QUIT");
}

int retroarch_toggleMenu(void)
{
    return retroarch_cmd("MENU_TOGGLE");
}

int retroarch_getInfo(RetroArchInfo_t *info)
{
    char response[128];
    if (retroarch_get("GET_INFO", response, sizeof(response)) == -1) {
        return -1;
    }

    // Parse response "GET_INFO %d %d NO" or "GET_INFO %d %d %d"
    int parsed = sscanf(response, "GET_INFO %u %u %d", &info->max_disk_slots, &info->disk_slot, &info->state_slot);

    if (parsed < 2) {
        return -1;
    }

    info->has_state_slot = parsed == 3;

    return 0;
}
