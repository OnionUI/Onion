#ifndef TWEAKS_NETWORK_H__
#define TWEAKS_NETWORK_H__

#include <SDL/SDL_image.h>
#include <ctype.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "components/kbinput_wrapper.h"
#include "components/list.h"
#include "components/types.h"
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

#define INITIAL_CAPACITY 10 // preallocate for 10 network slots
#define GROWTH_FACTOR 2     // gets doubled if we hit 10
#define WIFI_CONNECT_WAIT 7000
#define PSK_MIN_LENGTH 8
#define PSK_MAX_LENGTH 63
#define SSID_MAX_LENGTH 32
#define WIFI_SCAN_WAIT 3            // how long to wait for a wifi scan in seconds
#define WIFI_CONNECT_INITIAL_WAIT 4 // how long to wait before checking for a wifi connect in seconds
#define WIFI_CONNECT_MAX_WAIT 6     // how long to wait additionally to the initial wait in seconds
#define WIFI_CONNECT_SLEEP 50000    /* how long to sleep between wifi connect checks in microseconds */
                                    /* needs to be fast to not miss the "DISCONNECTED" state         */

typedef struct { // create a struct to store the wifi networks, we can use access them afterwards if we need to for anything
    WifiNetwork *networks;
    int count; // keep track of the count
} WifiNetworkList;

WifiNetworkList globalNetworkList; // define global struct

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
    bool vncserv;
    bool loaded;
    int vncfps;
} network_state = {
    .vncfps = 20,
};

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
    network_state.vncserv = config_flag_get(".vncServer");
    config_get(".vncfps", CONFIG_INT, &network_state.vncfps);
    network_state.loaded = true;
}

static Share *_network_shares = NULL;
static int network_numShares;

WifiNetwork *_wifi_networks = NULL;
static int network_numWifiNetworks;

static bool reset_wifi = false;
pthread_mutex_t wifi_scan_mutex = PTHREAD_MUTEX_INITIALIZER;
bool wifi_scan_running = false;

// forward decs
void network_scanWifiNetworks();
void network_freeWifiNetworks()
{
    if (globalNetworkList.networks != NULL) {
        free(globalNetworkList.networks);
        globalNetworkList.networks = NULL;
    }
    globalNetworkList.count = 0;
}

// checks for null character in a string; is backwards! (call with !).
// called by network_handleParsedNetwork.
bool network_ssidContainsNull(char *str, int len)
{
    /* TODO fix this. test with hidden wifis
    for (int i = 0; i < len; i++) {
        if (str[i] == '\x00') {
            return true;
        }
    }*/
    return false;
}

// wpa_cli escapes " and \ in the SSID with a backslash.
void unescape(char *str)
{
    if (str == NULL || strlen(str) == 0)
        return;

    int len = strlen(str);
    int i, j = 0;

    for (i = 0; i < len; i++) {
        if (str[i] == '\\' && i < len - 1) {
            switch (str[i + 1]) {
            case '\\':
                str[j++] = '\\';
                i++;
                break;
            case '"':
                str[j++] = '"';
                i++;
                break;
            default:
                str[j++] = '\\';
                break;
            }
        }
        else {
            str[j++] = str[i];
        }
    }

    str[j] = '\0';
}

// TODO move these out of here maybe
void escapeForShell(char *str)
{
    int i, j;
    int newLength = strlen(str) * 2;
    char *result = (char *)malloc(newLength);

    for (i = 0, j = 0; i < strlen(str); i++) {
        if (str[i] == '\'') {
            result[j++] = '\'';
            result[j++] = '\\';
            result[j++] = '\'';
            result[j++] = '\'';
        }
        else {
            result[j++] = str[i];
        }
    }

    result[j] = '\0';
    strcpy(str, result);
    free(result);
}
void deleteNetwork(const char *ssid)
{
    char cmd[STR_MAX];
    char escaped_ssid[65];
    strcpy(escaped_ssid, ssid);
    escapeForShell(escaped_ssid);
    sprintf(cmd, "%s/wifi.sh delete \'%s\' &", NET_SCRIPT_PATH, escaped_ssid);
    printf("deleting network %s\nwith cmd %s\n", escaped_ssid, cmd);
    system(cmd);
}
// the actual connection
void connectToWiFi(const char *ssid, const char *psk)
{
    char cmd[STR_MAX];
    char escaped_ssid[65];
    strcpy(escaped_ssid, ssid);
    escapeForShell(escaped_ssid);

    if (psk) {
        char escaped_psk[128];
        strcpy(escaped_psk, psk);
        escapeForShell(escaped_psk);
        snprintf(cmd, STR_MAX, "%s/wifi.sh connect \'%s\' \'%s\' &", NET_SCRIPT_PATH, escaped_ssid, escaped_psk);
    }
    else
        snprintf(cmd, STR_MAX, "%s/wifi.sh connect \'%s\' &", NET_SCRIPT_PATH, escaped_ssid);

    system(cmd);
    char message[STR_MAX];
    sprintf(message, "Connecting to WiFi network\n%s", ssid);
    showInfoDialogGeneric("WiFi networks", message, false, 0);
    sleep(WIFI_CONNECT_INITIAL_WAIT);

    struct timeval t1, t2;
    int elapsedTime;
    gettimeofday(&t1, NULL);
    while (true) {
        gettimeofday(&t2, NULL);
        elapsedTime = (t2.tv_sec - t1.tv_sec);

        if (elapsedTime > WIFI_CONNECT_MAX_WAIT) {
            // when using a wrong password the 4 way handshake can take long
            // so we need to delete the network
            deleteNetwork(ssid);
            showInfoDialogGeneric("WiFi networks", "WiFi connection timed out.\nPress any button to continue", true, 0);
            return;
        }

        char output[STR_MAX];
        process_start_read_return(NET_SCRIPT_PATH "/wifi.sh wpa_state", output);
        if (strcmp(output, "COMPLETED") == 0) {
            showInfoDialogGeneric("WiFi networks", "Connection successful", false, 1000);
            return;
        }
        else if (strcmp(output, "DISCONNECTED") == 0) {
            deleteNetwork(ssid);
            showInfoDialogGeneric("WiFi networks", "Failed to connect to WiFi network.\nPress any button to continue", true, 0);
            return;
        }
        usleep(WIFI_CONNECT_SLEEP);
    }
}

// starts WiFi connection
// called in the menu action
void network_connectWifi(void *pt)
{
    WifiNetwork *wifi_network = (WifiNetwork *)(((ListItem *)pt)->payload_ptr);
    if (wifi_network->connected)
        return;

    if (wifi_network->known) {
        connectToWiFi(wifi_network->ssid, NULL);
    }
    else {
        if (wifi_network->encrypted) {
            const char *psk = launch_keyboard("", "Enter your WiFi password");
            if (strlen(psk) < PSK_MIN_LENGTH || strlen(psk) > PSK_MAX_LENGTH) {
                showInfoDialogGeneric("WiFi networks", "Password must be 8 to 63 characters.\nPress any button to continue", true, 0);
                return;
            }
            else {
                connectToWiFi(wifi_network->ssid, psk);
            }
        }
        else {
            connectToWiFi(wifi_network->ssid, NULL);
        }
    }
    all_changed = true;
    reset_menus = true;
    network_scanWifiNetworks();
}

// parses a line to extract network data from `scan_results`
// called by network_parseAndPopulate.
bool network_parseLine(const char *line, WifiNetwork *network)
{
    int ret = sscanf(line, "%18s\t%d\t%d\t%255s\t%64s[^\n]",
                     network->bssid,
                     &network->frequency,
                     &network->signal_level,
                     network->flags,
                     network->ssid);
    unescape(network->ssid);
    return ret == 5;
}

// sets network flags based on parsed data.
// called by network_parseAndPopulate.
void network_handleParsedNetwork(WifiNetwork *network, const char *current_ssid)
{
    if (network_ssidContainsNull(network->ssid, SSID_MAX_LENGTH)) {
        strcpy(network->ssid, "Bad name");
    }

    if (strstr(network->flags, "WPA") != NULL ||
        strstr(network->flags, "WEP") != NULL)
        network->encrypted = true;
    else
        network->encrypted = false;

    if (strcmp(current_ssid, network->ssid) == 0)
        network->connected = true;
    else
        network->connected = false;

    char cmd[STR_MAX * 2];
    char escaped_ssid[STR_MAX];
    strcpy(escaped_ssid, network->ssid);
    escapeForShell(escaped_ssid);
    sprintf(cmd, "%s/wifi.sh known \'%s\' &", NET_SCRIPT_PATH, escaped_ssid);
    char output[STR_MAX];
    process_start_read_return(cmd, output);

    if (strlen(output) > 0) {
        network->known = true;
        network->id = atoi(output);
    }
    else {
        network->known = false;
        network->id = -1;
    }
}

// parses and populates global network list / struct from file pointer.
// called by network_scanAndPop.
void network_parseAndPopulate(FILE *fp, const char *current_ssid)
{
    char line[STR_MAX];
    int capacity = INITIAL_CAPACITY;
    globalNetworkList.networks = (WifiNetwork *)calloc(capacity, sizeof(WifiNetwork));
    if (globalNetworkList.networks == NULL) {
        perror("Failed to allocate memory");
        return;
    }
    globalNetworkList.count = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (globalNetworkList.count >= capacity) {
            capacity *= GROWTH_FACTOR;
            WifiNetwork *new_networks = (WifiNetwork *)realloc(globalNetworkList.networks, capacity * sizeof(WifiNetwork));
            if (new_networks == NULL) {
                perror("Failed to reallocate memory");
                free(globalNetworkList.networks);
                globalNetworkList.count = 0;
                return;
            }
            globalNetworkList.networks = new_networks;
        }

        WifiNetwork *current_network = &globalNetworkList.networks[globalNetworkList.count];
        if (network_parseLine(line, current_network)) {
            network_handleParsedNetwork(current_network, current_ssid);
            globalNetworkList.count++;
        }
        else {
            strcpy(current_network->ssid, "Hidden");
            printf("HIDDEN NETWORK %s", line);
            globalNetworkList.count++;
        }
    }
}

// starts network scan, processes results with process_start_read_return, populates global network list with network_parseAndPopulate.
// called by network_getWifiNetworks.
void network_scanAndPop()
{
    network_freeWifiNetworks();

    char current_ssid[SSID_MAX_LENGTH + 1] = "";
    char cmd[STR_MAX];
    sprintf(cmd, "%s/wifi.sh connected &", NET_SCRIPT_PATH);
    process_start_read_return(cmd, current_ssid);
    unescape(current_ssid);

    system("wpa_cli scan > /dev/null");
    sleep(WIFI_SCAN_WAIT);

    FILE *fp = popen("wpa_cli scan_result | tail -n +3", "r");
    if (fp == NULL) {
        perror("Failed to get scan results");
        return;
    }

    network_parseAndPopulate(fp, current_ssid);

    pclose(fp);
}

int compareSignalLevel(const void *a, const void *b)
{
    WifiNetwork *wifiA = (WifiNetwork *)a;
    WifiNetwork *wifiB = (WifiNetwork *)b;
    if (wifiA->signal_level < wifiB->signal_level)
        return 1;
    else if (wifiA->signal_level > wifiB->signal_level)
        return -1;
    else
        return 0;
}

void *network_getWifiNetworks()
{
    if (pthread_mutex_trylock(&wifi_scan_mutex) != 0)
        return NULL;
    wifi_scan_running = true;
    printf("[WiFi] scan thread START %ld\n", syscall(__NR_gettid));

    WifiNetworkList networkList = {0};
    network_scanAndPop(&networkList);

    _wifi_networks = networkList.networks;
    network_numWifiNetworks = networkList.count;

    list_changed = true;
    reset_menus = true;
    all_changed = true;
    reset_wifi = true;

    printf("[WiFi] scan thread END %ld\n", syscall(__NR_gettid));
    wifi_scan_running = false;
    pthread_mutex_unlock(&wifi_scan_mutex);
    return NULL;
}

// thread function to update wifi scanning text in UI during scan.
// called by network_scanWifiNetworks.
void *network_updateScanningLabel(void *pt)
{
    ListItem *item = (ListItem *)pt;
    int id = item->_id;
    const char *labelVariations[] = {"Scanning", "Scanning.", "Scanning..", "Scanning..."};
    const int numVariations = sizeof(labelVariations) / sizeof(labelVariations[0]);
    int counter = 0;

    while (wifi_scan_running) {
        strcpy(_menu_wifi.items[id].label, labelVariations[counter]);
        counter = (counter + 1) % numVariations;
        list_changed = true;
        usleep(500000);
    }
    return NULL;
}

// starts wifi scanning in new threads for scanning new networks, non blocking as threaded
// called by action men u item
void network_scanWifiNetworks(void *pt)
{
    network_numWifiNetworks = 0;
    _wifi_networks = NULL;
    wifi_scan_running = true;
    pthread_t network_updateScanningLabel_thread;
    pthread_create(&network_updateScanningLabel_thread, NULL, network_updateScanningLabel, pt);
    pthread_detach(network_updateScanningLabel_thread);

    pthread_t wifi_thread;
    pthread_create(&wifi_thread, NULL, network_getWifiNetworks, NULL);
    pthread_detach(wifi_thread);
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
    char state[STR_MAX];
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
    char authed[STR_MAX];
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
    char cmd[STR_MAX];
    sprintf(cmd, "%s/wifi.sh %s &", NET_SCRIPT_PATH, wifion ? "on" : "off");
    system(cmd);

    reset_wifi = true;
    reset_menus = true;
    all_changed = true;
}

void network_toggleVNC(void *pt)
{
    char command_start[STR_MAX];
    char command_stop[STR_MAX];

    int new_fps = (int)network_state.vncfps;

    sprintf(command_start, "/mnt/SDCARD/.tmp_update/bin/vncserver -k /dev/input/event0 -F %d -r 180 > /dev/null 2>&1 &", new_fps);
    sprintf(command_stop, "killall -9 vncserver");

    if (!network_state.vncserv) {
        network_state.vncserv = true;
        network_setState(&network_state.vncserv, ".vncServer", true);
        reset_menus = true;
        if (!process_isRunning("vncserver")) {
            system(command_start);
        }
    }
    else {
        network_state.vncserv = false;
        network_setState(&network_state.vncserv, ".vncServer", false);
        reset_menus = true;
        if (process_isRunning("vncserver")) {
            system(command_stop);
        }
    }
}

void network_setVNCFPS(void *pt)
{
    network_state.vncfps = ((ListItem *)pt)->value;
    config_setNumber(".vncfps", network_state.vncfps);

    if (network_state.vncserv) {
        network_toggleVNC(pt);
        network_state.vncserv = false;
        network_setState(&network_state.vncserv, ".vncServer", false);
        reset_menus = true;
    }
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
        printf("##### Creating ssh menu\n");
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

void menu_vnc(void *pt)
{
    ListItem *item = (ListItem *)pt;
    item->value = (int)network_state.vncserv;
    if (!_menu_vnc._created) {
        _menu_vnc = list_create(2, LIST_SMALL);
        strcpy(_menu_vnc.title, "VNC");
        list_addItem(&_menu_vnc,
                     (ListItem){
                         .label = "Enable",
                         .item_type = TOGGLE,
                         .value = (int)network_state.vncserv,
                         .action = network_toggleVNC});
        list_addItemWithInfoNote(&_menu_vnc,
                                 (ListItem){
                                     .label = "Framerate",
                                     .item_type = MULTIVALUE,
                                     .value_max = 20,
                                     .value_min = 1,
                                     .value = (int)network_state.vncfps,
                                     .action = network_setVNCFPS},
                                 "Set the framerate of the VNC server\n"
                                 "between 1 and 20. The higher the \n"
                                 "framerate the more CPU it will use \n");
    }
    menu_stack[++menu_level] = &_menu_vnc;
    header_changed = true;
}

void menu_wifi(void *pt)
{
    ListItem *item = (ListItem *)pt;
    item->value = (int)settings.wifi_on;

    if (reset_wifi) {
        list_free(&_menu_wifi);
    }

    if (!_menu_wifi._created) {
        _menu_wifi = list_createWithTitle(4 + globalNetworkList.count, LIST_SMALL, "WiFi networks"); // use networkList->count instead of MAX_NUM_WIFI_NETWORKS so we're not limited to 35
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
        list_addItem(&_menu_wifi,
                     (ListItem){
                         .label = "Add a hidden WiFi network...",
                         .action = action_addHiddenNetwork});
        list_addItem(&_menu_wifi,
                     (ListItem){
                         .label = "Scan for networks",
                         .disabled = !settings.wifi_on,
                         .action = network_scanWifiNetworks});

        // sort before displaying
        qsort(globalNetworkList.networks, globalNetworkList.count, sizeof(WifiNetwork), compareSignalLevel);

        for (int i = 0; i < globalNetworkList.count; i++) {
            ListItem wifi_network = {
                .item_type = WIFINETWORK,
                .payload_ptr = globalNetworkList.networks + i,
                .action = network_connectWifi};
            snprintf(wifi_network.label, STR_MAX - 1, "%s", globalNetworkList.networks[i].ssid);
            list_addItem(&_menu_wifi, wifi_network);
        }
        list_changed = true;
        all_changed = true;
        reset_wifi = false;
    }
    menu_stack[++menu_level] = &_menu_wifi;
    header_changed = true;
}

void menu_network(void *_)
{
    if (!_menu_network._created) {
        _menu_network = list_create(10, LIST_SMALL);
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
                                     .label = "VNC: Screen share...",
                                     .item_type = TOGGLE,
                                     .disabled = !settings.wifi_on,
                                     .alternative_arrow_action = true,
                                     .arrow_action = network_toggleVNC,
                                     .value = (int)network_state.vncserv,
                                     .action = menu_vnc},
                                 "Connect to your MMP from another device\n"
                                 "to view the screen and interact with it.");
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
