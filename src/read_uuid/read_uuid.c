#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 255

void remove_0x_and_newline(char *str)
{
    char *src = str;
    char *dest = str;
    while (*src) {
        if (src[0] == '0' && src[1] == 'x') {
            src += 2;
        }
        else if (src[0] == '\n') {
            src++;
        }
        else {
            *dest = *src;
            dest++;
            src++;
        }
    }
    *dest = '\0';
}

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
            free(output);
        }
        else {
            fprintf(stderr, "Error executing command: %s\n", commands[i]);
            exit(1);
        }
    }
    remove_0x_and_newline(serial);
    printf("%s\n", serial);
    return 0;
}
