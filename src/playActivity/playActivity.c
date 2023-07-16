#include "./playActivity.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void printUsage()
{
    printf("Usage: playActivity start [rom_path] -> Launch the counter for this rom\n");
    printf("       playActivity stop [rom_path]  -> Stop the counter for this rom\n");
    printf("       playActivity migrate          -> Migrate the old database (prior to Onion 4.2.0) to SQLite\n");
}

int main(int argc, char *argv[])
{
    log_setName("play_activity");

    if (argc < 2) {
        printUsage();
        return EXIT_SUCCESS;
    }

    if (strcmp(argv[1], "start") == 0) {
        if (argc >= 3) {
            play_activity_start(argv[2]);
        }
        else {
            printf("Error: Missing rom_path argument\n");
            printUsage();
        }
    }
    else if (strcmp(argv[1], "stop") == 0) {
        if (argc >= 3) {
            play_activity_stop(argv[2]);
        }
        else {
            printf("Error: Missing rom_path argument\n");
            printUsage();
        }
    }
    else if (strcmp(argv[1], "migrate") == 0) {
        migrateDB();
    }
    else {
        printf("Error: Invalid argument '%s'\n", argv[1]);
        printUsage();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
