#include <stdio.h>
#include <string.h>
#include <utils/hash.h>

//
//  Takes data from stdin and hashes it using the FNV1A_Pippip_Yurii algorithm
//
int main()
{
    size_t buffer_size = 1024;
    char *input_buffer = malloc(buffer_size);

    if (input_buffer == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        return 1;
    }

    size_t total_size = 0;
    size_t bytesRead;

    // Read input from stdin until EOF
    while ((bytesRead = fread(input_buffer + total_size, 1,
                              buffer_size - total_size, stdin)) > 0) {
        total_size += bytesRead;

        // Check if the buffer is full, resize it if needed
        if (total_size == buffer_size) {
            buffer_size *= 2;
            input_buffer = realloc(input_buffer, buffer_size);

            if (input_buffer == NULL) {
                fprintf(stderr, "Memory reallocation error\n");
                free(input_buffer);
                return 1;
            }
        }
    }

    input_buffer[total_size] = '\0';

    int hash = FNV1A_Pippip_Yurii(input_buffer, strlen(input_buffer));
    printf("%u\n", hash);

    free(input_buffer);
    return 0;
}