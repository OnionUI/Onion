#ifndef UTILS_NETINFO_H__
#define UTILS_NETINFO_H__

#include <arpa/inet.h>
#include <errno.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "log.h"
#include "str.h"

bool netinfo_getIpAddress(char *label_out, const char *interface)
{
    char ip_address[STR_MAX];
    int n = socket(AF_INET, SOCK_DGRAM, 0);
    struct ifreq ifr;

    // Type of address to retrieve - IPv4 IP address
    ifr.ifr_addr.sa_family = AF_INET;

    // Copy the interface name in the ifreq structure
    strncpy(ifr.ifr_name, interface, IFNAMSIZ - 1);
    ioctl(n, SIOCGIFADDR, &ifr);
    close(n);

    snprintf(ip_address, STR_MAX - 1, "IP address: %s (%s)", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), interface);

    if (strncmp(ip_address, label_out, STR_MAX) != 0) {
        strcpy(label_out, ip_address);
        return true;
    }
    return false;
}

void netinfo_getHostnameAndIpAddress(char *hostname, char *ip_address)
{
    char host_buffer[STR_MAX];
    char *ip_buffer;

    print_debug("Start");

    // Retrieve hostname
    if (gethostname(host_buffer, sizeof(host_buffer)) != -1) {
        printf_debug("Hostname: %s\n", host_buffer);
        sprintf(hostname, "Hostname: %s", host_buffer);

        // To retrieve host information
        struct hostent *host_entry = gethostbyname(host_buffer);

        if (host_entry != NULL) {
            // Convert IP address to ASCII string
            ip_buffer = inet_ntoa(*((struct in_addr *)host_entry->h_addr_list[0]));
            sprintf(ip_address, "IP address: %s", ip_buffer);
            printf_debug("IP address: %s\n", ip_address);
        }
        else {
            print_debug("Failed to retrieve IP address");
            strncpy(ip_address, "IP address: -", STR_MAX - 1);
        }
    }
    else {
        print_debug("Failed to retrieve hostname");
        strncpy(hostname, "Hostname: -", STR_MAX - 1);
    }

    print_debug("Done");
}

#endif // UTILS_NETINFO_H__
