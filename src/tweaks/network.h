#ifndef TWEAKS_NETWORK_H__
#define TWEAKS_NETWORK_H__

#include <SDL/SDL_image.h>
#include <ctype.h>
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
#include "utils/netinfo.h"

#include "./appstate.h"

#define NET_SCRIPT_PATH "/mnt/SDCARD/.tmp_update/script/network"
#define SMBD_CONFIG_PATH "/mnt/SDCARD/.tmp_update/config/smb.conf"
#define MAX_NUM_WIFI_NETWORKS 32 /*  Can't really find a way around this right now  */
                                 /*  but it really shouldn't be an issue            */
                                 /*  since the list is sorted by signal stength     */
static struct network_s {
    bool smbd;
    bool http;
    bool ssh;
    bool telnet;
    bool ftp;
    bool hotspot;
    bool ntp;
    bool ntp_wait;
    bool auth_smbd;
    bool auth_ftp;
    bool auth_http;
    bool auth_ssh;
    bool manual_tz;
    bool check_updates;
    bool keep_alive;
    bool loaded;
} network_state;

void network_loadState(void)
{
    if (network_state.loaded)
        return;
    network_state.smbd = config_flag_get(".smbdState");
    network_state.http = config_flag_get(".httpState");
    network_state.ssh = config_flag_get(".sshState");
    network_state.telnet = config_flag_get(".telnetState");
    network_state.ftp = config_flag_get(".ftpState");
    network_state.hotspot = config_flag_get(".hotspotState");
    network_state.ntp = config_flag_get(".ntpState");
    network_state.ntp_wait = config_flag_get(".ntpWait");
    network_state.auth_ftp = config_flag_get(".authftpState");
    network_state.auth_http = config_flag_get(".authhttpState");
    network_state.auth_ssh = config_flag_get(".authsshState");
    network_state.manual_tz = config_flag_get(".manual_tz");
    network_state.check_updates = config_flag_get(".checkUpdates");
    network_state.keep_alive = config_flag_get(".keepServicesAlive");
    network_state.loaded = true;
}

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
} WifiNetwork;

static Share *_network_shares = NULL;
static int network_numShares;

WifiNetwork *_wifi_networks = NULL;
static int network_numWifiNetworks;

void network_freeWifiNetworks()
{
    if (_wifi_networks != NULL)
        free(_wifi_networks);
}

void network_getWifiNetworks()
{

    system("rm -f /tmp/wifi.list && wpa_cli scan > /dev/null");
    sleep(5);
    system("wpa_cli scan_result | tail -n +3 > /tmp/wifi.list");
    system("sync");

    FILE *file = fopen("/tmp/wifi.list", "r");
    if (file == NULL) {
        perror("Failed to open the wifi list");
        return;
    }

    char line[256];
    _wifi_networks = (WifiNetwork *)calloc(1, sizeof(WifiNetwork));
    system("cat /tmp/wifi.list\n\n"); // TODO: remove
    network_numWifiNetworks = 0;

    while (fgets(line, sizeof(line), file) != NULL) {
        if (network_numWifiNetworks == MAX_NUM_WIFI_NETWORKS)
            break;

        if (network_numWifiNetworks > 0)
            _wifi_networks = (WifiNetwork *)realloc(_wifi_networks,
                                                    (network_numWifiNetworks + 1) * sizeof(WifiNetwork));
        if (sscanf(line, "%18s\t%d\t%d\t%255s\t%32[^\n]",
                   _wifi_networks[network_numWifiNetworks].bssid,
                   &_wifi_networks[network_numWifiNetworks].frequency,
                   &_wifi_networks[network_numWifiNetworks].signal_level,
                   _wifi_networks[network_numWifiNetworks].flags,
                   _wifi_networks[network_numWifiNetworks].ssid) == 5) {
            if (strstr(_wifi_networks[network_numWifiNetworks].flags, "WPA") != NULL ||
                strstr(_wifi_networks[network_numWifiNetworks].flags, "WEP") != NULL) {
                _wifi_networks[network_numWifiNetworks].encrypted = true;
            }
            else {
                _wifi_networks[network_numWifiNetworks].encrypted = false;
            }
            network_numWifiNetworks++;
        }
        else {
            printf("Failed to parse the line: %s", line); // it fails on hidden wifis.. which is kinda the point?
        }
    }
    fclose(file);
    for (int i = 0; i < network_numWifiNetworks; i++) {

        ListItem wifi_network = {
            .item_type = ACTION};
        snprintf(wifi_network.label, STR_MAX - 1, "SSID: %s  encrypted: %d  SIG:%d",
                 _wifi_networks[i].ssid, _wifi_networks[i].encrypted, _wifi_networks[i].signal_level);
        list_addItem(&_menu_wifi, wifi_network);
        all_changed = true;
        // printf("SSID: %s\tSignal Strength: %d\tKey Type: %s\n", _wifi_networks[i].ssid, _wifi_networks[i].signal_strength, _wifi_networks[i].key_type);
    }
}

void network_freeSmbShares()
{
    if (_network_shares != NULL) {
        free(_network_shares);
    }
}

void network_getSmbShares()
{
    if (_network_shares != NULL) {
        return;
    }

    int numShares = 0;

    FILE *file = fopen(SMBD_CONFIG_PATH, "r");
    if (file == NULL) {
        printf("Failed to open smb.conf file.\n");
        return;
    }

    char line[STR_MAX];

    bool found_shares = false;
    bool is_available = false;
    long availablePos = -1;

    while (fgets(line, sizeof(line), file) != NULL) {
        char *trimmedLine = strtok(line, "\n");
        if (trimmedLine == NULL) {
            continue;
        }

        if (strstr(trimmedLine, "available = ") != NULL) {
            availablePos = ftell(file) - strlen(trimmedLine) - 1;
            is_available = strstr(trimmedLine, "1") != NULL;
            continue;
        }

        if (strstr(trimmedLine, "path = ") != NULL) {
            strncpy(_network_shares[numShares - 1].path, trimmedLine + 7, STR_MAX);
            continue;
        }

        if (strncmp(trimmedLine, "[", 1) == 0 && strncmp(trimmedLine + strlen(trimmedLine) - 1, "]", 1) == 0) {
            if (found_shares) {
                _network_shares[numShares - 1].available = is_available;
                _network_shares[numShares - 1].availablePos = availablePos;
                is_available = false;
            }

            char *shareName = strtok(trimmedLine + 1, "]");
            if (shareName != NULL && strlen(shareName) > 0) {
                if (strcmp(shareName, "global") == 0) {
                    continue;
                }

                numShares++;
                _network_shares = (Share *)realloc(_network_shares, numShares * sizeof(Share));

                bool add_exclamation = false;
                if (strncmp("__", shareName, 2) == 0) {
                    shareName = shareName + 2;
                    add_exclamation = true;
                }

                strncpy(_network_shares[numShares - 1].name, shareName, STR_MAX - 11);

                if (add_exclamation) {
                    strncat(_network_shares[numShares - 1].name, " (!)", STR_MAX - 11 - strlen(shareName));
                }

                found_shares = true;
            }
        }
    }

    if (found_shares) {
        _network_shares[numShares - 1].available = is_available;
        _network_shares[numShares - 1].availablePos = availablePos;
    }

    network_numShares = numShares;

    fclose(file);
}

void network_toggleSmbAvailable(void *item)
{
    ListItem *listItem = (ListItem *)item;
    Share *share = (Share *)listItem->payload_ptr;

    FILE *file = fopen(SMBD_CONFIG_PATH, "r+");
    if (file == NULL) {
        printf("Failed to open smb.conf file.\n");
        return;
    }

    if (fseek(file, share->availablePos, SEEK_SET) != 0) {
        printf("Failed to seek to the available property of share '%s'.\n", share->name);
        fclose(file);
        return;
    }

    char line[STR_MAX];
    fgets(line, sizeof(line), file);
    share->available = strstr(line, "1") == NULL; // toggle

    fseek(file, share->availablePos, SEEK_SET);
    fprintf(file, "available = %d\n", share->available);
    fflush(file);

    fclose(file);
}

void network_setState(bool *state_ptr, const char *flag_name, bool value)
{
    *state_ptr = value;
    config_flag_set(flag_name, value);
}

void network_execServiceState(const char *service_name, bool background)
{
    char state[256];
    char command[512];

    sync();

    sprintf(state, NET_SCRIPT_PATH "/update_networking.sh %s toggle", service_name);
    sprintf(command, "%s 2>&1", state);
    if (background)
        strcat(command, " &");
    system(command);

    printf_debug("network_execServiceState: %s\n", state);
}

void network_execServiceAuth(const char *service_name)
{
    char authed[256];
    char command[512];

    sync();

    sprintf(authed, NET_SCRIPT_PATH "/update_networking.sh %s authed", service_name);
    sprintf(command, "%s 2>&1", authed);

    system(command);

    printf_debug("network_execServiceAuth: %s\n", authed);
}

void network_commonEnableToggle(List *list, ListItem *item, bool *value_pt, const char *service_name, const char *service_flag)
{
    bool enabled = item->value == 1;
    network_setState(value_pt, service_flag, enabled);
    if (_menu_network._created) {
        list_currentItem(&_menu_network)->value = enabled;
    }
    network_execServiceState(service_name, false);
    reset_menus = true;
    all_changed = true;
}

void network_setSmbdState(void *pt)
{
    network_commonEnableToggle(&_menu_smbd, (ListItem *)pt, &network_state.smbd, "smbd", ".smbdState");
}

void network_setHttpState(void *pt)
{
    network_commonEnableToggle(&_menu_http, (ListItem *)pt, &network_state.http, "http", ".httpState");
}

void network_setSshState(void *pt)
{
    network_commonEnableToggle(&_menu_ssh, (ListItem *)pt, &network_state.ssh, "ssh", ".sshState");
}

void network_setFtpState(void *pt)
{
    network_commonEnableToggle(&_menu_ftp, (ListItem *)pt, &network_state.ftp, "ftp", ".ftpState");
}

void network_setTelnetState(void *pt)
{
    network_commonEnableToggle(&_menu_telnet, (ListItem *)pt, &network_state.telnet, "telnet", ".telnetState");
}

void network_setHotspotState(void *pt)
{
    network_setState(&network_state.hotspot, ".hotspotState", ((ListItem *)pt)->value);
    network_execServiceState("hotspot", false);
    reset_menus = true;
    all_changed = true;
}

void network_setNtpState(void *pt)
{
    network_setState(&network_state.ntp, ".ntpState", ((ListItem *)pt)->value);
    temp_flag_set("ntp_synced", false);
    network_execServiceState("ntp", true);
    reset_menus = true;
    all_changed = true;
}

void network_setNtpWaitState(void *pt)
{
    network_setState(&network_state.ntp_wait, ".ntpWait", ((ListItem *)pt)->value);
}

void network_setCheckUpdates(void *pt)
{
    network_setState(&network_state.check_updates, ".checkUpdates", ((ListItem *)pt)->value);
}

void network_keepServicesAlive(void *pt)
{
    network_setState(&network_state.keep_alive, ".keepServicesAlive", !((ListItem *)pt)->value);
}

void network_setFtpAuthState(void *pt)
{
    network_setState(&network_state.auth_ftp, ".authftpState", ((ListItem *)pt)->value);
    network_execServiceAuth("ftp");
}

void network_setHttpAuthState(void *pt)
{
    network_setState(&network_state.auth_http, ".authhttpState", ((ListItem *)pt)->value);
    network_execServiceAuth("http");
}

void network_setSshAuthState(void *pt)
{
    network_setState(&network_state.auth_ssh, ".authsshState", ((ListItem *)pt)->value);
    network_execServiceAuth("ssh");
}

void network_wpsConnect(void *pt)
{
    system("sh " NET_SCRIPT_PATH "/wpsclient.sh");
}

void network_setTzManualState(void *pt)
{
    bool enabled = ((ListItem *)pt)->value;
    network_setState(&network_state.manual_tz, ".manual_tz", !enabled);
    if (enabled) {
        char utc_str[10];
        if (config_get(".tz_sync", CONFIG_STR, utc_str)) {
            setenv("TZ", utc_str, 1);
            tzset();
            config_setString(".tz", utc_str);
        }
        else {
            temp_flag_set("ntp_synced", false);
        }
    }
    reset_menus = true;
    all_changed = true;
}

void network_setTzSelectState(void *pt)
{
    char utc_str[10];
    int select_value = ((ListItem *)pt)->value;
    double utc_value = ((double)select_value / 2.0) - 12.0;
    bool half_past = round(utc_value) != utc_value;

    if (utc_value == 0.0) {
        strcpy(utc_str, "UTC");
    }
    else {
        // UTC +/- is reversed for export TZ
        sprintf(utc_str, utc_value > 0 ? "UTC-%02d:%02d" : "UTC+%02d:%02d", (int)floor(abs(utc_value)), half_past ? 30 : 0);
    }

    printf_debug("Set timezone: %s\n", utc_str);

    setenv("TZ", utc_str, 1);
    tzset();
    config_setString(".tz", utc_str);
}

void network_toggleWifi(void *pt)
{
    bool wifion = ((ListItem *)pt)->value;
    settings.wifi_on = wifion;
    char cmd[256];
    // sprintf(cmd, "%s/wifi.sh %d &", NET_SCRIPT_PATH, wifion);
    system(cmd);

    if (menu_stack[menu_level] == &_menu_wifi) {
        if (wifion) {
            //showInfoDialogGeneric("WiFi networks", "Scanning for networks...\n", false);
            //network_getWifiNetworks();
        }
    }
    reset_menus = true;
    all_changed = true;
    list_changed = true;
}

void menu_smbd(void *pt)
{
    ListItem *item = (ListItem *)pt;
    item->value = (int)network_state.smbd;

    if (!_menu_smbd._created) {
        network_getSmbShares();

        _menu_smbd = list_createWithSticky(1 + network_numShares, "Samba");

        list_addItemWithInfoNote(&_menu_smbd,
                                 (ListItem){
                                     .label = "Enable",
                                     .sticky_note = "Enable Samba file sharing",
                                     .item_type = TOGGLE,
                                     .value = (int)network_state.smbd,
                                     .action = network_setSmbdState},
                                 item->info_note);

        for (int i = 0; i < network_numShares; i++) {
            ListItem shareItem = {
                .item_type = TOGGLE,
                .disabled = !network_state.smbd,
                .action = network_toggleSmbAvailable, // set the action to the wrapper function
                .value = _network_shares[i].available,
                .payload_ptr = _network_shares + i // store a pointer to the share in the payload
            };
            snprintf(shareItem.label, STR_MAX - 1, "Share: %s", _network_shares[i].name);
            strncpy(shareItem.sticky_note, str_replace(_network_shares[i].path, "/mnt/SDCARD", "SD:"), STR_MAX - 1);
            list_addItem(&_menu_smbd, shareItem);
        }
    }

    menu_stack[++menu_level] = &_menu_smbd;
    header_changed = true;
}

void menu_http(void *pt)
{
    ListItem *item = (ListItem *)pt;
    item->value = (int)network_state.http;
    if (!_menu_http._created) {
        _menu_http = list_create(2, LIST_SMALL);
        strcpy(_menu_http.title, "HTTP");
        list_addItemWithInfoNote(&_menu_http,
                                 (ListItem){
                                     .label = "Enable",
                                     .item_type = TOGGLE,
                                     .value = (int)network_state.http,
                                     .action = network_setHttpState},
                                 item->info_note);
        list_addItemWithInfoNote(&_menu_http,
                                 (ListItem){
                                     .label = "Enable authentication",
                                     .item_type = TOGGLE,
                                     .disabled = !network_state.http,
                                     .value = (int)network_state.auth_http,
                                     .action = network_setHttpAuthState},
                                 "Username: admin\n"
                                 "Password: admin\n"
                                 " \n"
                                 "It's recommended you change this\n"
                                 "at first login.");
    }
    menu_stack[++menu_level] = &_menu_http;
    header_changed = true;
}

void menu_ftp(void *pt)
{
    ListItem *item = (ListItem *)pt;
    item->value = (int)network_state.ftp;
    if (!_menu_ftp._created) {
        _menu_ftp = list_create(2, LIST_SMALL);
        strcpy(_menu_ftp.title, "FTP");
        list_addItemWithInfoNote(&_menu_ftp,
                                 (ListItem){
                                     .label = "Enable",
                                     .item_type = TOGGLE,
                                     .value = (int)network_state.ftp,
                                     .action = network_setFtpState},
                                 item->info_note);
        list_addItemWithInfoNote(&_menu_ftp,
                                 (ListItem){
                                     .label = "Enable authentication",
                                     .item_type = TOGGLE,
                                     .disabled = !network_state.ftp,
                                     .value = (int)network_state.auth_ftp,
                                     .action = network_setFtpAuthState},
                                 "Username: onion\n"
                                 "Password: onion\n"
                                 " \n"
                                 "We're using a new auth system. User defined\n"
                                 "passwords will come in a future update.");
    }
    menu_stack[++menu_level] = &_menu_ftp;
    header_changed = true;
}

void menu_ssh(void *pt)
{
    ListItem *item = (ListItem *)pt;
    item->value = (int)network_state.ssh;
    if (!_menu_ssh._created) {
        _menu_ssh = list_create(2, LIST_SMALL);
        strcpy(_menu_ssh.title, "SSH");
        list_addItemWithInfoNote(&_menu_ssh,
                                 (ListItem){
                                     .label = "Enable",
                                     .item_type = TOGGLE,
                                     .value = (int)network_state.ssh,
                                     .action = network_setSshState},
                                 item->info_note);
        list_addItemWithInfoNote(&_menu_ssh,
                                 (ListItem){
                                     .label = "Enable authentication",
                                     .item_type = TOGGLE,
                                     .disabled = !network_state.ssh,
                                     .value = (int)network_state.auth_ssh,
                                     .action = network_setSshAuthState},
                                 "Username: onion\n"
                                 "Password: onion\n"
                                 " \n"
                                 "We're using a new auth system. User defined\n"
                                 "passwords will come in a future update.");
    }
    menu_stack[++menu_level] = &_menu_ssh;
    header_changed = true;
}

void menu_wifi(void *pt)
{
    ListItem *item = (ListItem *)pt;
    item->value = (int)settings.wifi_on;

    if (_menu_wifi._created)
        list_free(&_menu_wifi);

    _menu_wifi = list_createWithTitle(2 + MAX_NUM_WIFI_NETWORKS, LIST_SMALL, "WiFi networks");
    list_addItem(&_menu_wifi,
                 (ListItem){
                     .label = "Enabled",
                     .item_type = TOGGLE,
                     .value = (int)settings.wifi_on,
                     .action = network_toggleWifi});
    list_addItem(&_menu_wifi,
                 (ListItem){
                     .label = "WPS connect",
                     .disabled = !settings.wifi_on,
                     .action = network_wpsConnect});
    if (settings.wifi_on) {
        showInfoDialogGeneric("WiFi networks", "Scanning for networks...\n", false);
        network_getWifiNetworks();
    }
    menu_stack[++menu_level] = &_menu_wifi;
    header_changed = true;
}

void menu_network(void *_)
{
    if (!_menu_network._created) {
        _menu_network = list_create(9, LIST_SMALL);
        strcpy(_menu_network.title, "Network");

        network_loadState();

        list_addItem(&_menu_network,
                     (ListItem){
                         .label = "IP address: N/A",
                         .disabled = true,
                         .action = NULL});
        list_addItemWithInfoNote(&_menu_network,
                                 (ListItem){
                                     .label = "WiFi...",
                                     .item_type = TOGGLE,
                                     .alternative_arrow_action = true,
                                     .arrow_action = network_toggleWifi,
                                     .value = (int)settings.wifi_on,
                                     .action = menu_wifi},
                                 "Enable or disable WiFi.\n"
                                 "Connect to a network using\n"
                                 "a password or WPS.\n");
        list_addItemWithInfoNote(&_menu_network,
                                 (ListItem){
                                     .label = "WiFi Hotspot",
                                     .item_type = TOGGLE,
                                     .disabled = !settings.wifi_on,
                                     .value = (int)network_state.hotspot,
                                     .action = network_setHotspotState},
                                 "Use hotspot to host all the network\n"
                                 "services on the go, no router needed.\n"
                                 "Stay connected at anytime, anywhere.\n"
                                 "Compatible with Easy Netplay and\n"
                                 "regular netplay.");
        list_addItemWithInfoNote(&_menu_network,
                                 (ListItem){
                                     .label = "Samba: Network file share...",
                                     .item_type = TOGGLE,
                                     .disabled = !settings.wifi_on,
                                     .alternative_arrow_action = true,
                                     .arrow_action = network_setSmbdState,
                                     .value = (int)network_state.smbd,
                                     .action = menu_smbd},
                                 "Samba is a file sharing protocol that provides\n"
                                 "integrated sharing of files and directories\n"
                                 "between your Miyoo Mini Plus and your PC.\n"
                                 " \n"
                                 "Username: onion\n"
                                 "Password: onion\n");
        list_addItemWithInfoNote(&_menu_network,
                                 (ListItem){
                                     .label = "HTTP: Web-based file sync...",
                                     .item_type = TOGGLE,
                                     .disabled = !settings.wifi_on,
                                     .alternative_arrow_action = true,
                                     .arrow_action = network_setHttpState,
                                     .value = (int)network_state.http,
                                     .action = menu_http},
                                 "HTTP file server allows you to manage your\n"
                                 "files through a web browser on your phone,\n"
                                 "PC or tablet.\n"
                                 " \n"
                                 "Think of it as a website hosted by Onion,\n"
                                 "simply enter the IP address in your browser.");
        list_addItemWithInfoNote(&_menu_network,
                                 (ListItem){
                                     .label = "SSH: Secure shell...",
                                     .item_type = TOGGLE,
                                     .disabled = !settings.wifi_on,
                                     .alternative_arrow_action = true,
                                     .arrow_action = network_setSshState,
                                     .value = (int)network_state.ssh,
                                     .action = menu_ssh},
                                 "SSH provides a secure command line host\n"
                                 "for communicating with your device remotely.\n"
                                 " \n"
                                 "SFTP provides a secure file transfer protocol.");
        list_addItemWithInfoNote(&_menu_network,
                                 (ListItem){
                                     .label = "FTP: File server...",
                                     .item_type = TOGGLE,
                                     .disabled = !settings.wifi_on,
                                     .alternative_arrow_action = true,
                                     .arrow_action = network_setFtpState,
                                     .value = (int)network_state.ftp,
                                     .action = menu_ftp},
                                 "FTP provides a method of transferring files\n"
                                 "between Onion and a PC, phone, or tablet.\n"
                                 "You'll need an FTP client installed on the\n"
                                 "other device.");
        list_addItemWithInfoNote(&_menu_network,
                                 (ListItem){
                                     .label = "Telnet: Remote shell",
                                     .item_type = TOGGLE,
                                     .disabled = !settings.wifi_on,
                                     .value = (int)network_state.telnet,
                                     .action = network_setTelnetState},
                                 "Telnet provides unencrypted remote shell\n"
                                 "access to your device.");
        list_addItemWithInfoNote(&_menu_network,
                                 (ListItem){
                                     .label = "Disable services in game",
                                     .item_type = TOGGLE,
                                     .value = !network_state.keep_alive,
                                     .action = network_keepServicesAlive},
                                 "Disable all network services (except WiFi)\n"
                                 "while playing games.\n"
                                 " \n"
                                 "This helps to conserve battery and\n"
                                 "to keep performance at a maximum.");
    }
    strcpy(_menu_network.items[0].label, ip_address_label);
    menu_stack[++menu_level] = &_menu_network;
    header_changed = true;
}

#endif // TWEAKS_NETWORK_H__
