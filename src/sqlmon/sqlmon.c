#include "sqlmon.h"
#include "sqlmon_db.h"

void strip_newline(char *str)
{
    if (!str)
        return;
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[len - 1] = '\0';
        len--;
    }
}

void write_fd(const char *msg, int fd)
{
    if (write(fd, msg, strlen(msg)) < 0)
        print_debug("Unable to write message to socket\n");

    fsync(fd);
}

static void handle_client(void *fd_ptr)
{
    int client_fd = *(int *)fd_ptr;
    free(fd_ptr);
    char buffer[BUF_SIZE];
    int bytes = read(client_fd, buffer, BUF_SIZE - 1);
    if (bytes <= 0) {
        close(client_fd);
        return;
    }
    printf_debug("Command received from client %s\n", buffer);
    buffer[bytes] = '\0';
    strip_newline(buffer);
    char *msg;

    if (strncmp(buffer, GET_PLAY_TIME_TOTAL, strlen(GET_PLAY_TIME_TOTAL)) == 0) {
        int play_time = play_activity_get_total_play_time();
        msg = (char *)(malloc(sizeof(char) * 25));
        if (!msg)
            print_debug("Unable to alloc string for play time\n");
        else if (snprintf(msg, 25, "%d", play_time) < 0)
            print_debug("Unable to transform int into string\n");
    }
    else if (strncmp(buffer, GET_PLAY_TIME, strlen(GET_PLAY_TIME)) == 0) {
        int play_time = play_activity_get_play_time(&buffer[strlen(GET_PLAY_TIME) + 1]);
        msg = (char *)(malloc(sizeof(char) * 25));
        if (!msg)
            print_debug("Unable to alloc string for play time\n");
        else if (snprintf(msg, 25, "%d", play_time) < 0) {
            print_debug("Unable to transform int into string\n");
        }
    }
    else if (strncmp(buffer, STOP_ALL_PLAY_ACTIVITY, strlen(STOP_ALL_PLAY_ACTIVITY)) == 0) {
        msg = strdup("OK");
        play_activity_stop_all();
    }
    else if (strncmp(buffer, STOP_PLAY_ACTIVITY, strlen(STOP_PLAY_ACTIVITY)) == 0) {
        bool ok = play_activity_stop(&buffer[strlen(STOP_PLAY_ACTIVITY) + 1]);
        if (ok)
            msg = strdup("OK");
        else
            msg = strdup("ERR");
    }
    else if (strncmp(buffer, START_PLAY_ACTIVITY, strlen(START_PLAY_ACTIVITY)) == 0) {
        bool ok = play_activity_start(&buffer[strlen(START_PLAY_ACTIVITY) + 1]);
        if (ok)
            msg = strdup("OK");
        else
            msg = strdup("ERR");
    }
    else if (strncmp(buffer, RESUME_PLAY_ACTIVITY, strlen(RESUME_PLAY_ACTIVITY)) == 0) {
        bool ok = play_activity_resume();
        if (ok)
            msg = strdup("OK");
        else
            msg = strdup("ERR");
    }
    else if (strncmp(buffer, CLOSE_DB, strlen(CLOSE_DB)) == 0) {
        msg = strdup("OK");
        quit = 1;
    }
    else
        msg = "Unknown command\n";

    printf_debug("Command sent to client %s\n", msg);
    write_fd(msg, client_fd);
    free(msg);

    close(client_fd);
}

int main(int argc, char *argv[])
{

    log_setName("sqldaemon");
    print_debug("\n\nDebug logging enabled");

    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);
    quit = init();

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        print_debug("Unable to alloc a socket for the play activity daemon\n");
        sql_shutdown();
        return EXIT_FAILURE;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {

        print_debug("Unable to start bind socket of the play activity daemon\n");
        return EXIT_FAILURE;
    }
    if (listen(server_fd, 5) == -1) {
        print_debug("Unable to start listening\n");
        sql_shutdown();
        return EXIT_FAILURE;
    }

    printf_debug("PlayActivity SQLite daemon running on port %d\n", PORT);

    while (quit) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0)
            continue;

        pthread_t thread_id;
        int *fd_ptr = malloc(sizeof(int));
        if (!fd_ptr) {
            print_debug("Unable to alloc int for new thread");
            close(client_fd);
            continue;
        }
        *fd_ptr = client_fd;
        pthread_create(&thread_id, NULL, (void *)handle_client, (void *)fd_ptr);
        pthread_detach(thread_id);
    }

    if (server_fd >= 0) {
        close(server_fd);
    }
    sql_shutdown();
    return EXIT_SUCCESS;
}
