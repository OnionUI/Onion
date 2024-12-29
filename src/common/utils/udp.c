#include "udp.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

static const char *UDP_DEFAULT_IP = "127.0.0.1"; // localhost
static const int UDP_DEFAULT_PORT = 55355;       // default RetroArch CMD port

static int _init_socket(const char *ipAddress, int port, int *socket_fd, struct sockaddr_in *server_address)
{
    *socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (*socket_fd == -1) {
        perror("Socket creation failed");
        return -1;
    }

    memset(server_address, 0, sizeof(*server_address));
    server_address->sin_family = AF_INET;
    server_address->sin_port = htons(port);
    if (inet_pton(AF_INET, ipAddress, &server_address->sin_addr) <= 0) {
        perror("Invalid IP address");
        close(*socket_fd);
        return -1;
    }

    return 0;
}

int udp_send(const char *ipAddress, int port, const char *message)
{
    int socket_fd;
    struct sockaddr_in server_address;

    if (_init_socket(ipAddress, port, &socket_fd, &server_address) == -1) {
        return -1;
    }

    if (sendto(socket_fd, message, strlen(message), 0, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Failed to send data");
        close(socket_fd);
        return -1;
    }

    close(socket_fd);
    return 0;
}

int udp_send_receive(const char *ipAddress, int port, const char *message, char *response, size_t response_size)
{
    int socket_fd;
    struct sockaddr_in server_address;

    if (_init_socket(ipAddress, port, &socket_fd, &server_address) == -1) {
        return -1;
    }

    if (sendto(socket_fd, message, strlen(message), 0, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Failed to send data");
        close(socket_fd);
        return -1;
    }

    ssize_t bytes_received = recvfrom(socket_fd, response, response_size, 0, NULL, NULL);
    if (bytes_received == -1) {
        perror("Failed to receive data");
        close(socket_fd);
        return -1;
    }

    response[bytes_received] = '\0'; // Null-terminate the received string

    close(socket_fd);
    return 0;
}

int retroarch_cmd(const char *cmd)
{
    return udp_send(UDP_DEFAULT_IP, UDP_DEFAULT_PORT, cmd);
}

int retroarch_get(const char *cmd, char *response, size_t response_size)
{
    return udp_send_receive(UDP_DEFAULT_IP, UDP_DEFAULT_PORT, cmd, response, response_size);
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
