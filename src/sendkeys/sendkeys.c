#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>

#include "utils/log.h"

int main(int argc, char *argv[])
{
	struct input_event events[100];

	if (argc < 3 || argc % 2 == 0) {
        printf("Usage: sendkeys [[CODE] [VALUE], ...]\nValues: 0 - released, 1 - pressed, 2 - repeating\n");
        return 1;
    }

	int num_events = (argc - 1) / 2;

	for (int i = 1; i < argc; i += 2) {
		int ev_idx = (i - 1) / 2;
		events[ev_idx].type = EV_KEY;
		events[ev_idx].code = atoi(argv[i]);
		events[ev_idx].value = atoi(argv[i + 1]);
	}

	int input_fd;

	for (int j = 0; j < num_events; j++) {
		printf_debug("sendkeys: code = %d, value = %d\n", events[j].code, events[j].value);
		input_fd = open("/dev/input/event0", O_WRONLY);
		write(input_fd, &events[j], sizeof(events[j]));
		close(input_fd);
		sync();
	}

	return 0;
}
