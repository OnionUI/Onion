#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    char *ipAddress = "127.0.0.1"; // localhost
    int port = 55355;              // default RetroArch CMD port
    char *message;                 // the message to send

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <MESSAGE> [<IP> <PORT>]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    message = argv[1];

    if (argc == 4) {
        ipAddress = argv[2];
        port = atoi(argv[3]);
    }

    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    if (inet_pton(AF_INET, ipAddress, &server_address.sin_addr) <= 0) {
        perror("Invalid IP address");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    ssize_t bytes_sent = sendto(socket_fd, message, strlen(message), 0,
                                (struct sockaddr *)&server_address, sizeof(server_address));

    if (bytes_sent == -1) {
        perror("Failed to send data");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    printf("Sent %d bytes to %s:%d\n", bytes_sent, ipAddress, port);

    close(socket_fd);

    return EXIT_SUCCESS;
}
