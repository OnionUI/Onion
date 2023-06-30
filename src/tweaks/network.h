#ifndef TWEAKS_NETWORK_H__
#define TWEAKS_NETWORK_H__

#include <SDL/SDL_image.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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
    network_setServiceState(service_name);
    reset_menus = true;
    all_changed = true;
}

void network_setHttpState(void *pt)
{
    network_commonEnableToggle(&_menu_http, (ListItem *)pt, &settings.http_state, "http", ".httpState");
}

void network_setSshState(void *pt)
{
    network_commonEnableToggle(&_menu_ssh, (ListItem *)pt, &settings.ssh_state, "ssh", ".sshState");
}

void network_setFtpState(void *pt)
{
    network_commonEnableToggle(&_menu_ftp, (ListItem *)pt, &settings.ftp_state, "ftp", ".ftpState");
}

void network_setTelnetState(void *pt)
{
    network_commonEnableToggle(&_menu_telnet, (ListItem *)pt, &settings.telnet_state, "telnet", ".telnetState");
}

void network_setHotspotState(void *pt)
{
    config_flag_set(".hotspotState", ((ListItem *)pt)->value == 1);
    settings.hotspot_state = ((ListItem *)pt)->value == 1;
    network_setServiceState("hotspot");
}

void network_setNtpState(void *pt)
{
    config_flag_set(".ntpState", ((ListItem *)pt)->value == 1);
    settings.ntp_state = ((ListItem *)pt)->value == 1;
    network_setServiceState("ntp");
    reset_menus = true;
    all_changed = true;
}

void network_setNtpWaitState(void *pt)
{
    settings.ntp_wait = ((ListItem *)pt)->value == 1;
}

void network_setTelnetAuthState(void *pt)
{
    config_flag_set(".authtelnetState", ((ListItem *)pt)->value == 1);
    settings.auth_telnet_state = ((ListItem *)pt)->value == 1;
    network_setServiceAuth("telnet");
}

void network_setFtpAuthState(void *pt)
{
    config_flag_set(".authftpState", ((ListItem *)pt)->value == 1);
    settings.auth_ftp_state = ((ListItem *)pt)->value == 1;
    network_setServiceAuth("ftp");
}

void network_setHttpAuthState(void *pt)
{
    config_flag_set(".authhttpState", ((ListItem *)pt)->value == 1);
    settings.auth_http_state = ((ListItem *)pt)->value == 1;
    network_setServiceAuth("http");
}

void network_setSshAuthState(void *pt)
{
    config_flag_set(".authsshState", ((ListItem *)pt)->value == 1);
    settings.auth_ssh_state = ((ListItem *)pt)->value == 1;
    network_setServiceAuth("ssh");
}

void network_wpsConnect(void *pt)
{
    system("sh /mnt/SDCARD/.tmp_update/script/wpsclient.sh");
}

void network_setTzSelectState(void *pt)
{
    char utc_str[10];
    int select_value = ((ListItem *)pt)->value;
    int utc_value = select_value - 12;

    if (utc_value == 0) {
        strcpy(utc_str, "UTC");
    }
    else {
        // UTC +/- is reversed for export TZ
        sprintf(utc_str, utc_value > 0 ? "UTC-%d" : "UTC+%d", abs(utc_value));
    }

    printf_debug("Set timezone: %s\n", utc_str);

    setenv("TZ", utc_str, 1);
    tzset();
    config_setString(".tz", utc_str);

    config_setNumber("tzselect", select_value);
    settings.tzselect_state = select_value;

    temp_flag_set("timezone_update", true);
    sync();
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
                         .action = network_setHttpState});
        list_addItem(&_menu_http,
                     (ListItem){
                         .label = "Enable authentication",
                         .item_type = TOGGLE,
                         .disabled = !settings.http_state,
                         .value = (int)settings.auth_http_state,
                         .action = network_setHttpAuthState});
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
        strcpy(_menu_telnet.title, "Telnet protocol");
        list_addItem(&_menu_telnet,
                     (ListItem){
                         .label = "Enable",
                         .item_type = TOGGLE,
                         .value = (int)settings.telnet_state,
                         .action = network_setTelnetState});
        list_addItem(&_menu_telnet,
                     (ListItem){
                         .label = "Enable authentication",
                         .item_type = TOGGLE,
                         .disabled = !settings.telnet_state,
                         .value = (int)settings.auth_telnet_state,
                         .action = network_setTelnetAuthState});
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
        strcpy(_menu_ftp.title, "FTP server");
        list_addItem(&_menu_ftp,
                     (ListItem){
                         .label = "Enable",
                         .item_type = TOGGLE,
                         .value = (int)settings.ftp_state,
                         .action = network_setFtpState});
        list_addItem(&_menu_ftp,
                     (ListItem){
                         .label = "Enable authentication",
                         .item_type = TOGGLE,
                         .disabled = !settings.ftp_state,
                         .value = (int)settings.auth_ftp_state,
                         .action = network_setFtpAuthState});
    }
    menu_stack[++menu_level] = &_menu_ftp;
    header_changed = true;
}

void menu_wps(void *_)
{
    if (!_menu_wps._created) {
        _menu_wps = list_create(1, LIST_SMALL);
        strcpy(_menu_wps.title, "WPS control");
        list_addItem(&_menu_wps,
                     (ListItem){
                         .label = "WPS connect",
                         .action = network_wpsConnect});
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
        strcpy(_menu_ssh.title, "SSH protocol");
        list_addItem(&_menu_ssh,
                     (ListItem){
                         .label = "Enable",
                         .item_type = TOGGLE,
                         .value = (int)settings.ssh_state,
                         .action = network_setSshState});
        list_addItem(&_menu_ssh,
                     (ListItem){
                         .label = "Enable authentication",
                         .item_type = TOGGLE,
                         .disabled = !settings.ssh_state,
                         .value = (int)settings.auth_ssh_state,
                         .action = network_setSshAuthState});
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
                     (ListItem){
                         .label = "WiFi Hotspot",
                         .item_type = TOGGLE,
                         .value = (int)settings.hotspot_state,
                         .action = network_setHotspotState});
        list_addItem(&_menu_wifi,
                     (ListItem){
                         .label = "WPS...",
                         .action = menu_wps});
    }
    menu_stack[++menu_level] = &_menu_wifi;
    header_changed = true;
}

void menu_network(void *_)
{
    if (!_menu_network._created) {
        _menu_network = list_create(5, LIST_SMALL);
        strcpy(_menu_network.title, "Network");
        list_addItem(&_menu_network,
                     (ListItem){
                         .label = "WiFi...",
                         .action = menu_wifi});
        list_addItem(&_menu_network,
                     (ListItem){
                         .label = "HTTP File Server...",
                         .item_type = TOGGLE,
                         .disabled = !settings.wifi_on,
                         .alternative_arrow_action = 1,
                         .arrow_action = network_setHttpState,
                         .value = (int)settings.http_state,
                         .action = menu_http});
        list_addItem(&_menu_network,
                     (ListItem){
                         .label = "FTP server...",
                         .item_type = TOGGLE,
                         .disabled = !settings.wifi_on,
                         .alternative_arrow_action = 1,
                         .arrow_action = network_setFtpState,
                         .value = (int)settings.ftp_state,
                         .action = menu_ftp});
        list_addItem(&_menu_network,
                     (ListItem){
                         .label = "SSH protocol...",
                         .item_type = TOGGLE,
                         .disabled = !settings.wifi_on,
                         .alternative_arrow_action = 1,
                         .arrow_action = network_setSshState,
                         .value = (int)settings.ssh_state,
                         .action = menu_ssh});
        list_addItem(&_menu_network,
                     (ListItem){
                         .label = "Telnet protocol...",
                         .item_type = TOGGLE,
                         .disabled = !settings.wifi_on,
                         .alternative_arrow_action = 1,
                         .arrow_action = network_setTelnetState,
                         .value = (int)settings.telnet_state,
                         .action = menu_telnet});
    }
    menu_stack[++menu_level] = &_menu_network;
    header_changed = true;
}

#endif // TWEAKS_NETWORK_H__
