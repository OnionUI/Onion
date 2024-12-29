#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils/udp.h"

int main(int argc, char *argv[])
{
    char *ipAddress = "127.0.0.1"; // localhost
    int port = 55355;              // default RetroArch CMD port
    char *message;
    size_t response_size = 0;

    if (argc < 2 || (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0))) {
        fprintf(stderr, "Usage: %s [-h <ipAddress>] [-p <port>] [-r <response_size>] <message>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            if (i + 1 < argc) {
                ipAddress = argv[i + 1];
                i++;
            }
        }
        else if (strcmp(argv[i], "-p") == 0) {
            if (i + 1 < argc) {
                port = atoi(argv[i + 1]);
                i++;
            }
        }
        else if (strcmp(argv[i], "-r") == 0) {
            if (i + 1 < argc) {
                response_size = atoi(argv[i + 1]);
                i++;
            }
        }
        else {
            message = argv[i];
        }
    }

    if (response_size > 0) {
        char *response = (char *)malloc(response_size);
        if (response == NULL) {
            perror("Failed to allocate memory for response");
            exit(EXIT_FAILURE);
        }

        if (udp_send_receive(ipAddress, port, message, response, response_size) == -1) {
            exit(EXIT_FAILURE);
        }

        printf("%s\n", response);
        free(response);
    }
    else if (udp_send(ipAddress, port, message) == -1) {
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
