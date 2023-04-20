#include "./play_activity.h"

int main(int argc, char *argv[])
{
    log_setName("play_activity");
    if (strcmp(argv[1], "start") == 0) {
        play_activity_start(argv[2]);
    }
    if (strcmp(argv[1], "stop") == 0) {
        play_activity_stop(argv[2]);
    }
    return EXIT_SUCCESS;
}
