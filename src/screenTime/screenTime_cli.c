#include "screenTime.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include "utils/retroarch_cmd.h"

#define SCREEN_TIME_LIMIT_FLAG "/tmp/screen_time_limit_hit"
#define SCREEN_TIME_MONITOR_ENV_POLL_SECONDS "SCREEN_TIME_MONITOR_POLL_SECONDS"
#define SCREEN_TIME_MONITOR_ENV_ONCE "SCREEN_TIME_MONITOR_ONCE"
#define SCREEN_TIME_MONITOR_ENV_DRY_RUN "SCREEN_TIME_MONITOR_DRY_RUN"
#define SCREEN_TIME_MONITOR_DEFAULT_POLL_SECONDS 15
#define SCREEN_TIME_MONITOR_MIN_POLL_SECONDS 1
#define SCREEN_TIME_MONITOR_MAX_POLL_SECONDS 300

static void print_usage(void)
{
    printf("Usage: screenTime status\n"
           "       screenTime check [rom_path]\n"
           "       screenTime monitor [rom_path] [launcher_pid]\n"
           "       screenTime add-extra [minutes] [pin]\n"
           "       screenTime clear-extra [pin]\n"
           "       screenTime debug-set-remaining [seconds] [pin]\n"
           "       screenTime debug-clear [pin]\n"
           "       screenTime set-pin [new_pin] [current_pin]\n"
           "       screenTime clear-pin [current_pin]\n"
           "       screenTime verify-pin [pin]\n");
}

static bool env_enabled(const char *name)
{
    const char *value = getenv(name);
    return value != NULL && value[0] != '\0' && strcmp(value, "0") != 0;
}

static int monitor_poll_seconds(void)
{
    const char *value = getenv(SCREEN_TIME_MONITOR_ENV_POLL_SECONDS);
    if (value == NULL || value[0] == '\0')
        return SCREEN_TIME_MONITOR_DEFAULT_POLL_SECONDS;

    int seconds = atoi(value);
    if (seconds < SCREEN_TIME_MONITOR_MIN_POLL_SECONDS)
        return SCREEN_TIME_MONITOR_MIN_POLL_SECONDS;
    if (seconds > SCREEN_TIME_MONITOR_MAX_POLL_SECONDS)
        return SCREEN_TIME_MONITOR_MAX_POLL_SECONDS;
    return seconds;
}

static void touch_limit_flag(void)
{
    FILE *fp = fopen(SCREEN_TIME_LIMIT_FLAG, "w");
    if (fp == NULL)
        return;

    fputs("1", fp);
    fclose(fp);
}

static bool parse_pid(const char *value, pid_t *pid_out)
{
    if (value == NULL || value[0] == '\0' || pid_out == NULL)
        return false;

    char *end = NULL;
    errno = 0;
    long parsed = strtol(value, &end, 10);
    if (errno != 0 || end == value || parsed <= 0)
        return false;

    *pid_out = (pid_t)parsed;
    return true;
}

static bool is_pid_dir(const struct dirent *entry)
{
    for (const char *p = entry->d_name; *p != '\0'; p++) {
        if (!isdigit((unsigned char)*p))
            return false;
    }
    return true;
}

static bool process_parent_pid(pid_t pid, pid_t *ppid_out)
{
    char path[64];
    char stat_line[512];
    snprintf(path, sizeof(path), "/proc/%ld/stat", (long)pid);

    FILE *fp = fopen(path, "r");
    if (fp == NULL)
        return false;

    bool found = fgets(stat_line, sizeof(stat_line), fp) != NULL;
    fclose(fp);
    if (!found)
        return false;

    char *close_paren = strrchr(stat_line, ')');
    if (close_paren == NULL)
        return false;

    char state = '\0';
    long ppid = 0;
    if (sscanf(close_paren + 2, "%c %ld", &state, &ppid) != 2)
        return false;

    *ppid_out = (pid_t)ppid;
    return true;
}

static void signal_process_tree(pid_t root_pid, int sig)
{
    DIR *proc = opendir("/proc");
    if (proc != NULL) {
        struct dirent *entry;
        while ((entry = readdir(proc)) != NULL) {
            if (!is_pid_dir(entry))
                continue;

            pid_t pid = (pid_t)strtol(entry->d_name, NULL, 10);
            pid_t ppid = 0;
            if (pid > 0 && process_parent_pid(pid, &ppid) && ppid == root_pid)
                signal_process_tree(pid, sig);
        }
        closedir(proc);
    }

    kill(root_pid, sig);
}

static bool process_is_running(pid_t pid)
{
    return pid > 0 && kill(pid, 0) == 0;
}

static bool retroarch_is_running(void)
{
    return system("pidof retroarch > /dev/null") == 0;
}

static bool limit_target_is_running(pid_t launcher_pid)
{
    return process_is_running(launcher_pid) || retroarch_is_running();
}

static bool wait_for_limit_target_exit(pid_t launcher_pid, int timeout_seconds)
{
    for (int i = 0; i < timeout_seconds * 10; i++) {
        if (!limit_target_is_running(launcher_pid))
            return true;
        usleep(100000);
    }

    return !limit_target_is_running(launcher_pid);
}

static void enforce_limit(const char *rom_path, pid_t launcher_pid)
{
    touch_limit_flag();
    fprintf(stderr, "Screen time limit reached");
    if (rom_path != NULL && rom_path[0] != '\0')
        fprintf(stderr, ": %s", rom_path);
    fprintf(stderr, "\n");

    if (env_enabled(SCREEN_TIME_MONITOR_ENV_DRY_RUN))
        return;

    system("infoPanel --title \"Screen time\" --message \"Daily screen time limit reached.\" --auto &");

    retroarch_quit();
    if (wait_for_limit_target_exit(launcher_pid, 5))
        return;

    if (launcher_pid > 0)
        signal_process_tree(launcher_pid, SIGTERM);
    else
        system("killall -TERM retroarch");

    if (!wait_for_limit_target_exit(launcher_pid, 5)) {
        system("touch /tmp/.forceKillRetroarch");
        if (launcher_pid > 0)
            signal_process_tree(launcher_pid, SIGKILL);
        system("pidof retroarch > /dev/null && killall -9 retroarch");
    }
}

static int print_status(void)
{
    ScreenTimeStatus status;
    if (screen_time_get_status(&status) != 0) {
        fprintf(stderr, "Error: unable to read screen time status\n");
        return EXIT_FAILURE;
    }

    printf("enabled=%d\n", status.enabled ? 1 : 0);
    printf("usedSeconds=%" PRId64 "\n", status.used_seconds);
    printf("limitSeconds=%" PRId64 "\n", status.limit_seconds);
    printf("extraSeconds=%" PRId64 "\n", status.extra_seconds);
    if (status.remaining_seconds == INT64_MAX)
        printf("remainingSeconds=unlimited\n");
    else
        printf("remainingSeconds=%" PRId64 "\n", status.remaining_seconds);

    return EXIT_SUCCESS;
}

static int check_launch(void)
{
    ScreenTimeStatus status;
    if (screen_time_get_status(&status) != 0) {
        fprintf(stderr, "Error: unable to read screen time status\n");
        return EXIT_FAILURE;
    }

    if (screen_time_launch_allowed(&status)) {
        printf("allowed\n");
        return EXIT_SUCCESS;
    }

    printf("blocked: daily screen time limit reached\n");
    return 2;
}

static bool authorize_pin_arg(int argc, char *argv[], int pin_index)
{
    if (!screen_time_pin_configured())
        return true;

    if (argc <= pin_index || !screen_time_verify_pin(argv[pin_index])) {
        fprintf(stderr, "Error: invalid or missing PIN\n");
        return false;
    }

    return true;
}

static int monitor_limit(const char *rom_path, const char *launcher_pid_arg)
{
    int poll_seconds = monitor_poll_seconds();
    bool once = env_enabled(SCREEN_TIME_MONITOR_ENV_ONCE);
    pid_t launcher_pid = 0;
    parse_pid(launcher_pid_arg, &launcher_pid);

    while (true) {
        ScreenTimeStatus status;
        if (screen_time_get_status(&status) == 0) {
            if (!screen_time_launch_allowed(&status)) {
                enforce_limit(rom_path, launcher_pid);
                return 2;
            }
        }
        else {
            fprintf(stderr, "Warning: unable to read screen time status\n");
        }

        if (once)
            return EXIT_SUCCESS;

        sleep((unsigned int)poll_seconds);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        print_usage();
        return EXIT_SUCCESS;
    }

    if (strcmp(argv[1], "status") == 0)
        return print_status();

    if (strcmp(argv[1], "check") == 0)
        return check_launch();

    if (strcmp(argv[1], "monitor") == 0)
        return monitor_limit(argc >= 3 ? argv[2] : "", argc >= 4 ? argv[3] : "");

    if (strcmp(argv[1], "add-extra") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: missing minutes argument\n");
            return EXIT_FAILURE;
        }
        if (!authorize_pin_arg(argc, argv, 3))
            return 2;
        int minutes = atoi(argv[2]);
        if (screen_time_add_extra_minutes(minutes) != 0) {
            fprintf(stderr, "Error: unable to add extra time\n");
            return EXIT_FAILURE;
        }
        return print_status();
    }

    if (strcmp(argv[1], "clear-extra") == 0) {
        if (!authorize_pin_arg(argc, argv, 2))
            return 2;
        if (screen_time_clear_extra_minutes() != 0) {
            fprintf(stderr, "Error: unable to clear extra time\n");
            return EXIT_FAILURE;
        }
        return print_status();
    }

    if (strcmp(argv[1], "debug-set-remaining") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: missing seconds argument\n");
            return EXIT_FAILURE;
        }
        if (!authorize_pin_arg(argc, argv, 3))
            return 2;
        int seconds = atoi(argv[2]);
        if (screen_time_debug_set_remaining_seconds(seconds) != 0) {
            fprintf(stderr, "Error: unable to set debug remaining time\n");
            return EXIT_FAILURE;
        }
        return print_status();
    }

    if (strcmp(argv[1], "debug-clear") == 0) {
        if (!authorize_pin_arg(argc, argv, 2))
            return 2;
        if (screen_time_debug_clear_remaining() != 0) {
            fprintf(stderr, "Error: unable to clear debug remaining time\n");
            return EXIT_FAILURE;
        }
        return print_status();
    }

    if (strcmp(argv[1], "set-pin") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: missing pin argument\n");
            return EXIT_FAILURE;
        }
        if (!authorize_pin_arg(argc, argv, 3))
            return 2;
        return screen_time_set_pin(argv[2]) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    if (strcmp(argv[1], "clear-pin") == 0) {
        if (!authorize_pin_arg(argc, argv, 2))
            return 2;
        return screen_time_set_pin("") == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    if (strcmp(argv[1], "verify-pin") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: missing pin argument\n");
            return EXIT_FAILURE;
        }
        return screen_time_verify_pin(argv[2]) ? EXIT_SUCCESS : 2;
    }

    fprintf(stderr, "Error: invalid argument '%s'\n", argv[1]);
    print_usage();
    return EXIT_FAILURE;
}
