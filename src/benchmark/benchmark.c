#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

long long get_current_time_us()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000000 + tv.tv_usec;
}

void format_time_us_to_ms(char *buffer, long long time_us, int buffer_size)
{
    double time_ms = (double)time_us / 1000.0;
    snprintf(buffer, buffer_size, "%.3f", time_ms);
}

int main(int argc, char *argv[])
{

    char *old_bin;
    char *new_bin;
    int ITERATIONS = 100;
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (!strcmp("-i", argv[i])) {
                ITERATIONS = atoi(argv[++i]);
            }
            else if (!strcmp("-o", argv[i])) {
                asprintf(&old_bin, "%s >/dev/null 2>&1", argv[++i]);
            }
            else if (!strcmp("-n", argv[i])) {
                asprintf(&new_bin, "%s >/dev/null 2>&1", argv[++i]);
            }
        }
    }

    long long old_times[ITERATIONS];
    long long new_times[ITERATIONS];

    long long total_old_time = 0;
    long long total_new_time = 0;

    long long min_old_time = LLONG_MAX;
    long long max_old_time = 0;
    long long min_new_time = LLONG_MAX;
    long long max_new_time = 0;

    printf("Iterations: %d\n\n", ITERATIONS);

    // Run benchmark
    for (int i = 0; i < ITERATIONS; i++) {
        long long cycle_start = get_current_time_us();

        long long old_start = get_current_time_us();
        system(old_bin);
        long long old_end = get_current_time_us();
        old_times[i] = old_end - old_start;
        total_old_time += old_times[i];

        if (old_times[i] < min_old_time)
            min_old_time = old_times[i];
        if (old_times[i] > max_old_time)
            max_old_time = old_times[i];

        long long new_start = get_current_time_us();
        system(new_bin);
        long long new_end = get_current_time_us();
        new_times[i] = new_end - new_start;
        total_new_time += new_times[i];

        if (new_times[i] < min_new_time)
            min_new_time = new_times[i];
        if (new_times[i] > max_new_time)
            max_new_time = new_times[i];

        if ((i + 1) % 100 == 0) {
            printf("Completed %d iterations...\n", i + 1);
        }
    }

    printf("\nBenchmark Results\n");
    printf("==================\n\n");

    // Open DB statistics
    printf("Statistics old:\n");
    printf("  Total time: %lld us (%.3f ms)\n", total_old_time, (double)total_old_time / 1000.0);
    printf("  Average:    %.3f ms\n", (double)total_old_time / ITERATIONS / 1000.0);
    printf("  Min:        %lld us (%.3f ms)\n", min_old_time, (double)min_old_time / 1000.0);
    printf("  Max:        %lld us (%.3f ms)\n", max_old_time, (double)max_old_time / 1000.0);

    printf("\nStatistics new:\n");
    printf("  Total time: %lld us (%.3f ms)\n", total_new_time, (double)total_new_time / 1000.0);
    printf("  Average:    %.3f ms\n", (double)total_new_time / ITERATIONS / 1000.0);
    printf("  Min:        %lld us (%.3f ms)\n", min_new_time, (double)min_new_time / 1000.0);
    printf("  Max:        %lld us (%.3f ms)\n", max_new_time, (double)max_new_time / 1000.0);

    double average_old = (double)total_old_time / ITERATIONS / 1000;
    double average_new = (double)total_new_time / ITERATIONS / 1000;

    printf("  The new binary is    %.3f faster than the old binary\n", average_old / average_new);

    printf("\nBenchmark completed successfully.\n");

    return 0;
}
