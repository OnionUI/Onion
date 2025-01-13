#include "retroarch_cmd.h"

#include <stdio.h>
#include <string.h>

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

int retroarch_pause(void)
{
    return retroarch_cmd("PAUSE");
}

int retroarch_unpause(void)
{
    return retroarch_cmd("UNPAUSE");
}

int retroarch_getStateSlot(int *slot)
{
    char response[128];
    if (retroarch_get("GET_STATE_SLOT", response, sizeof(response)) == -1) {
        return -1;
    }

    // Parse response "GET_STATE_SLOT %d"
    return sscanf(response, "GET_STATE_SLOT %d", slot);
}

int retroarch_setStateSlot(int slot)
{
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "SET_STATE_SLOT %d", slot);
    return retroarch_cmd(cmd);
}

int retroarch_autosave(void)
{
    char response[128]; // Response is not used, but it is required by udp_send_receive
    return retroarch_get("SAVE_STATE_SLOT -1", response, sizeof(response));
}

int retroarch_save(int slot)
{
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "SAVE_STATE_SLOT %d", slot);
    return retroarch_cmd(cmd);
}

int retroarch_load(int slot)
{
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "LOAD_STATE_SLOT %d", slot);
    return retroarch_cmd(cmd);
}

int retroarch_getStatus(RetroArchStatus_s *status)
{
    char response[512];
    if (retroarch_get("GET_STATUS", response, sizeof(response)) == -1) {
        return -1;
    }

    char status_str[32];

    // Parse response "GET_STATUS PAUSED game_boy_advance,Advance Wars (U) (V1.1) [!],crc32=26fd0fc9"
    int parsed = sscanf(response, "GET_STATUS %31s %[^\n]", status_str, status->content_info);

    if (parsed < 2)
        strcpy(status->content_info, "");
    if (parsed < 1)
        return -1;

    if (strcmp(status_str, "PLAYING") == 0)
        status->state = RETROARCH_STATE_PLAYING;
    else if (strcmp(status_str, "PAUSED") == 0)
        status->state = RETROARCH_STATE_PAUSED;
    else if (strcmp(status_str, "CONTENTLESS") == 0)
        status->state = RETROARCH_STATE_CONTENTLESS;
    else
        status->state = RETROARCH_STATE_UNKNOWN;

    return 0;
}

int retroarch_getInfo(RetroArchInfo_s *info)
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
