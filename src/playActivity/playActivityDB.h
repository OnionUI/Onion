#ifndef PLAY_ACTIVITY_DB_H
#define PLAY_ACTIVITY_DB_H
#include "playActivityDBCommon.h"
#include "utils/log.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

int send_command(const char *cmd, char *buf)
{
    log_setName("play_activity");
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        print_debug("Unable to connect to socket\n");
        return 0;
    }
    if (write(sock, cmd, strlen(cmd)) == -1) {
        print_debug("Unable to write to socket\n");
        return 0;
    }

    int bytes;
    int total_bytes = 0;
    while ((bytes = read(sock, buf, BUF_SIZE - 1)) > 0) {
        total_bytes += bytes;
        if (total_bytes >= BUF_SIZE - 1) {
            print_debug("Server sent too much data");
            break;
        }
    }

    close(sock);
    buf[total_bytes] = '\0';
    printf_debug("Answer %s\n", buf);
    return total_bytes;
}

void play_activity_start(char *rom_file_path) {}
void play_activity_resume() {}
void play_activity_stop(char *rom_file_path) {}
void play_activity_stop_all() {}
void play_activity_db_close()
{

    char buf[BUF_SIZE];

    while (send_command(CLOSE_DB, buf) == 0) {
        continue;
    }
}
void play_activity_fix_paths() {}
void play_activity_list_all() {}
int play_activity_get_play_time(char *rom_path)
{
    char buf[BUF_SIZE];
    char *command;
    if (asprintf(&command, "%s %s", GET_PLAY_TIME, rom_path) < 0) {
        print_debug("Unable to alloc string for command\n");
        return 0;
    }

    int nbytes = send_command(command, buf);
    if (nbytes == 0 || nbytes == BUF_SIZE) {
        free(command);
        return 0;
    }

    char *endptr;
    int play_time = strtol(buf, &endptr, 10);
    if (endptr == buf)
        play_time = 0;

    free(command);
    return play_time;
}
int play_activity_get_total_play_time()
{
    char buf[BUF_SIZE];

    int nbytes = send_command(GET_PLAY_TIME_TOTAL, buf);
    if (nbytes == 0 || nbytes == BUF_SIZE) {
        return 0;
    }

    char *endptr;
    int play_time = strtol(buf, &endptr, 10);
    if (endptr == buf)
        play_time = 0;

    return play_time;
}
PlayActivitiesUI *play_activity_find_all(void)
{
    int count = 0;
    PlayActivitiesUI *entries = (PlayActivitiesUI *)(malloc(sizeof(PlayActivitiesUI) * count));
    entries->count = count;

    return entries;
}

#endif // PLAY_ACT&IVITY_DB_H
