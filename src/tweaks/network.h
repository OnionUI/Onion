#ifndef TWEAKS_NETWORK_H__
#define TWEAKS_NETWORK_H__

#include <SDL/SDL_image.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "components/list.h"
#include "system/keymap_sw.h"
#include "theme/render/dialog.h"
#include "theme/sound.h"
#include "utils/apply_icons.h"
#include "utils/json.h"
#include "utils/keystate.h"

#include "./appstate.h"

#define NET_SCRIPT_PATH "/mnt/SDCARD/.tmp_update/script/network"

void network_setServiceState(const char *service_name)
{
    char state[256];
    char command[512];

    sync();

    sprintf(state, NET_SCRIPT_PATH "/update_networking.sh %s toggle", service_name);
    sprintf(command, "%s 2>&1", state);

    system(command);

    printf_debug("network_setServiceState: %s\n", state);
}

void network_setServiceAuth(const char *service_name)
{
    char authed[256];
    char command[512];

    sync();

    sprintf(authed, NET_SCRIPT_PATH "/update_networking.sh %s authed", service_name);
    sprintf(command, "%s 2>&1", authed);

    system(command);

    printf_debug("network_setServiceAuth: %s\n", authed);
}

void network_commonEnableToggle(List *list, ListItem *item, bool *value_pt, const char *service_name, const char *service_flag)
{
    bool enabled = item->value == 1;
    config_flag_set(service_flag, enabled);
    *value_pt = enabled;
    if (_menu_network._created) {
        list_currentItem(&_menu_network)->value = enabled;
    }
    list_hideAllExcept(list, item, !enabled);
    network_setServiceState(service_name);
}

void action_sethttpstate(void *pt)
{
    network_commonEnableToggle(&_menu_http, (ListItem *)pt, &settings.http_state, "http", ".httpState");
}

void action_setsshstate(void *pt)
{
    network_commonEnableToggle(&_menu_ssh, (ListItem *)pt, &settings.ssh_state, "ssh", ".sshState");
}

void action_setftpstate(void *pt)
{
    network_commonEnableToggle(&_menu_ftp, (ListItem *)pt, &settings.ftp_state, "ftp", ".ftpState");
}

void action_settelnetstate(void *pt)
{
    network_commonEnableToggle(&_menu_telnet, (ListItem *)pt, &settings.telnet_state, "telnet", ".telnetState");
}

void action_sethotspotstate(void *pt)
{
    config_flag_set(".hotspotState", ((ListItem *)pt)->value == 1);
    settings.hotspot_state = ((ListItem *)pt)->value == 1;
    network_setServiceState("hotspot");
}

void action_setntpstate(void *pt)
{
    config_flag_set(".ntpState", ((ListItem *)pt)->value == 1);
    settings.ntp_state = ((ListItem *)pt)->value == 1;
    network_setServiceState("ntp");
    reset_menus = true;
    all_changed = true;
}

void action_setntpwaitstate(void *pt)
{
    settings.ntp_wait = ((ListItem *)pt)->value == 1;
}

void action_settelnetauthstate(void *pt)
{
    config_flag_set(".authtelnetState", ((ListItem *)pt)->value == 1);
    settings.auth_telnet_state = ((ListItem *)pt)->value == 1;
    network_setServiceAuth("telnet");
}

void action_setftpauthstate(void *pt)
{
    config_flag_set(".authftpState", ((ListItem *)pt)->value == 1);
    settings.auth_ftp_state = ((ListItem *)pt)->value == 1;
    network_setServiceAuth("ftp");
}

void action_sethttpauthstate(void *pt)
{
    config_flag_set(".authhttpState", ((ListItem *)pt)->value == 1);
    settings.auth_http_state = ((ListItem *)pt)->value == 1;
    network_setServiceAuth("http");
}

void action_setsshauthstate(void *pt)
{
    config_flag_set(".authsshState", ((ListItem *)pt)->value == 1);
    settings.auth_ssh_state = ((ListItem *)pt)->value == 1;
    network_setServiceAuth("ssh");
}

void action_wpsconnection(void *pt)
{
    system("sh /mnt/SDCARD/.tmp_update/script/wpsclient.sh");
}

void network_disableAll(void *pt)
{
    settings.http_state = ((ListItem *)pt)->value == 1;
    settings.ssh_state = ((ListItem *)pt)->value == 1;
    settings.ftp_state = ((ListItem *)pt)->value == 1;
    settings.telnet_state = ((ListItem *)pt)->value == 1;
    settings.hotspot_state = ((ListItem *)pt)->value == 1;
    settings.auth_telnet_state = ((ListItem *)pt)->value == 1;
    settings.auth_ftp_state = ((ListItem *)pt)->value == 1;
    settings.auth_ssh_state = ((ListItem *)pt)->value == 1;
    settings.auth_http_state = ((ListItem *)pt)->value == 1;
    system(NET_SCRIPT_PATH "/update_networking.sh disableall");
    reset_menus = true;
    all_changed = true;
}

void action_settzselectstate(void *pt)
{
    config_setNumber("tzselect", ((ListItem *)pt)->value);
    settings.tzselect_state = ((ListItem *)pt)->value;
    network_setServiceState("ntp");
}

void menu_http(void *pt)
{
    ListItem *item = (ListItem *)pt;
    item->value = (int)settings.http_state;
    if (!_menu_http._created) {
        _menu_http = list_create(2, LIST_SMALL);
        strcpy(_menu_http.title, "HTTP File Server");
        list_addItem(&_menu_http,
                     (ListItem){
                         .label = "Enable",
                         .item_type = TOGGLE,
                         .value = (int)settings.http_state,
                         .action = action_sethttpstate});
        list_addItem(&_menu_http,
                     (ListItem){
                         .label = "Enable authentication",
                         .item_type = TOGGLE,
                         .hidden = !settings.http_state,
                         .value = (int)settings.auth_http_state,
                         .action = action_sethttpauthstate});
    }
    menu_stack[++menu_level] = &_menu_http;
    header_changed = true;
}

void menu_telnet(void *pt)
{
    ListItem *item = (ListItem *)pt;
    item->value = (int)settings.telnet_state;
    if (!_menu_telnet._created) {
        _menu_telnet = list_create(2, LIST_SMALL);
        strcpy(_menu_telnet.title, "Telnet");
        list_addItem(&_menu_telnet,
                     (ListItem){
                         .label = "Enable",
                         .item_type = TOGGLE,
                         .value = (int)settings.telnet_state,
                         .action = action_settelnetstate});
        list_addItem(&_menu_telnet,
                     (ListItem){
                         .label = "Enable authentication",
                         .item_type = TOGGLE,
                         .hidden = !settings.telnet_state,
                         .value = (int)settings.auth_telnet_state,
                         .action = action_settelnetauthstate});
    }
    menu_stack[++menu_level] = &_menu_telnet;
    header_changed = true;
}

void menu_ftp(void *pt)
{
    ListItem *item = (ListItem *)pt;
    item->value = (int)settings.ftp_state;
    if (!_menu_ftp._created) {
        _menu_ftp = list_create(2, LIST_SMALL);
        strcpy(_menu_ftp.title, "FTP");
        list_addItem(&_menu_ftp,
                     (ListItem){
                         .label = "Enable",
                         .item_type = TOGGLE,
                         .value = (int)settings.ftp_state,
                         .action = action_setftpstate});
        list_addItem(&_menu_ftp,
                     (ListItem){
                         .label = "Enable authentication",
                         .item_type = TOGGLE,
                         .hidden = !settings.ftp_state,
                         .value = (int)settings.auth_ftp_state,
                         .action = action_setftpauthstate});
    }
    menu_stack[++menu_level] = &_menu_ftp;
    header_changed = true;
}

void menu_wps(void *_)
{
    if (!_menu_wps._created) {
        _menu_wps = list_create(1, LIST_SMALL);
        strcpy(_menu_wps.title, "WPS control");
        list_addItem(&_menu_wps, (ListItem){.label = "WPS connect",
                                            .action = action_wpsconnection});
    }
    menu_stack[++menu_level] = &_menu_wps;
    header_changed = true;
}

void menu_ssh(void *pt)
{
    ListItem *item = (ListItem *)pt;
    item->value = (int)settings.ssh_state;
    if (!_menu_ssh._created) {
        _menu_ssh = list_create(2, LIST_SMALL);
        strcpy(_menu_ssh.title, "SSH");
        list_addItem(&_menu_ssh, (ListItem){.label = "Enable",
                                            .item_type = TOGGLE,
                                            .value = (int)settings.ssh_state,
                                            .action = action_setsshstate});
        list_addItem(&_menu_ssh,
                     (ListItem){.label = "Enable authentication",
                                .item_type = TOGGLE,
                                .hidden = !settings.ssh_state,
                                .value = (int)settings.auth_ssh_state,
                                .action = action_setsshauthstate});
    }
    menu_stack[++menu_level] = &_menu_ssh;
    header_changed = true;
}

void menu_wifi(void *_)
{
    if (!_menu_wifi._created) {
        _menu_wifi = list_create(2, LIST_SMALL);
        strcpy(_menu_wifi.title, "WiFi");
        list_addItem(&_menu_wifi,
                     (ListItem){.label = "WiFi Hotspot",
                                .item_type = TOGGLE,
                                .value = (int)settings.hotspot_state,
                                .action = action_sethotspotstate});
        list_addItem(&_menu_wifi,
                     (ListItem){.label = "WPS...", .action = menu_wps});
    }
    menu_stack[++menu_level] = &_menu_wifi;
    header_changed = true;
}

void menu_networks(void *_)
{
    if (!_menu_network._created) {
        _menu_network = list_create(6, LIST_SMALL);
        strcpy(_menu_network.title, "Network");
        list_addItem(&_menu_network,
                     (ListItem){
                         .label = "WiFi...",
                         .action = menu_wifi});
        list_addItem(&_menu_network,
                     (ListItem){.label = "HTTP File Server...",
                                .item_type = TOGGLE,
                                .hidden = !settings.wifi_on,
                                .disable_arrows = 1,
                                .value = (int)settings.http_state,
                                .action = menu_http});
        list_addItem(&_menu_network,
                     (ListItem){.label = "SSH...",
                                .item_type = TOGGLE,
                                .hidden = !settings.wifi_on,
                                .disable_arrows = 1,
                                .value = (int)settings.ssh_state,
                                .action = menu_ssh});
        list_addItem(&_menu_network,
                     (ListItem){.label = "FTP...",
                                .item_type = TOGGLE,
                                .hidden = !settings.wifi_on,
                                .disable_arrows = 1,
                                .value = (int)settings.ftp_state,
                                .action = menu_ftp});
        list_addItem(&_menu_network,
                     (ListItem){.label = "Telnet...",
                                .item_type = TOGGLE,
                                .hidden = !settings.wifi_on,
                                .disable_arrows = 1,
                                .value = (int)settings.telnet_state,
                                .action = menu_telnet});
        list_addItem(&_menu_network,
                     (ListItem){.label = "Disable all", .action = network_disableAll});
    }
    menu_stack[++menu_level] = &_menu_network;
    header_changed = true;
}

#endif // TWEAKS_NETWORK_H__
