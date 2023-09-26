#include <fcntl.h>
#include <linux/input.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define KEY_ESC 1

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s program_name\n", argv[0]);
        return 1;
    }

    char command[256];
    snprintf(command, sizeof(command), "pkill -9 -f %s", argv[1]);

    int input_fd = open("/dev/input/event0", O_RDONLY);
    struct input_event event;

    while (read(input_fd, &event, sizeof(event)) == sizeof(event)) {
        if (event.type == EV_KEY && event.value == 1) {
            if (event.code == KEY_ESC) {
                system(command);
                break;
            }
        }
    }

    close(input_fd);
    return 0;
}
