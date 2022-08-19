#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>

int main(int argc, char *argv[])
{
	if (argc != 3) {
        printf("Usage: sendkeys [CODE] [VALUE]\nValues: 0 - released, 1 - pressed, 2 - repeating\n");
        return 1;
    }

	int input_fd = open("/dev/input/event0", O_WRONLY);
	struct input_event ev;

	ev.type = EV_KEY;
	ev.code = atoi(argv[1]);
	ev.value = atoi(argv[2]);

	write(input_fd, &ev, sizeof(ev));
	close(input_fd);
	sync();

	return 0;
}
