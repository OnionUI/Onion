#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 255

char *execute_command(const char *command)
{
    char buffer[MAX_LINE_LENGTH];
    char *result = NULL;

    FILE *pipe = popen(command, "r");
    if (pipe == NULL) {
        fprintf(stderr, "Error executing command: %s\n", command);
        exit(1);
    }

    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        result = strdup(buffer);
    }

    pclose(pipe);
    return result;
}

int main()
{
    char serial[22] = "";
    const char *commands[] = {
        "/config/riu_r 20 18 | awk 'NR==2'",
        "/config/riu_r 20 17 | awk 'NR==2'",
        "/config/riu_r 20 16 | awk 'NR==2'"};

    for (int i = 0; i < 3; i++) {
        char *output = execute_command(commands[i]);
        if (output != NULL) {
            strcat(serial, output);
        }
        else {
            fprintf(stderr, "Error executing command: %s\n", commands[i]);
            exit(1);
        }
    }

    // remove 0x and newlines
    int i, j;
    for (i = 0, j = 0; serial[i] != '\0'; i++) {
        if (serial[i] != '\n') {
            serial[j] = serial[i];
            j++;
        }
        if (serial[i] == '0' && serial[i + 1] == 'x') {
            i++;
        }
    }
    serial[j] = '\0';

    printf("%s\n", serial);
    return 0;
}
