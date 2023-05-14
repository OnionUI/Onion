#include "./playActivity.h"

void update_play_activity(const char *name, const char *relative_path)
{
    printf_debug("update_play_activity(%s, %s)\n", name, relative_path);
    FILE *file;
    time_t current_time;
    int play_time;
    file = fopen(INIT_TIMER_PATH, "rb");
    if (file == NULL) {
        file = fopen(INIT_TIMER_PATH, "wb");
        time(&current_time);
        play_time = (int)current_time;
        fwrite(&play_time, sizeof(int), 1, file);
        fclose(file);
    }
    else {
        fread(&play_time, sizeof(int), 1, file);
        time(&current_time);
        insert_data(name, relative_path, 1, ((int)current_time - play_time));
        play_time = (int)current_time;
        fseek(file, 0, SEEK_SET);
        fwrite(&play_time, sizeof(int), 1, file);
        fclose(file);
        remove(INIT_TIMER_PATH);
    }
    printf_debug("%s\n", "start_timer() return");
}

void usage(void) { printf_debug("%s\n", "main() argc <= 1"); }

int main(int argc, char *argv[])
{
    log_setName("playActivity");
    printf_debug("main(%d, %s)\n", argc, argv[1]);
    open_db();
    if (db == NULL) {
        printf_debug("%s\n", "db == NULL");
        return 1;
    }
    char *file_path = argv[1];
    char *roms_path = "../Roms/";
    char *relative_path = strstr(file_path, roms_path);
    if (relative_path == NULL) {
        printf_debug("'%s' must be in '%s' directory.\n", relative_path,
                     roms_path);
        return 1;
    }
    relative_path += strlen(roms_path);
    char *file_name = strrchr(relative_path, '/') + 1;
    char *extension = strrchr(file_name, '.');
    if (extension != NULL) {
        *extension = '\0';
    }
    update_play_activity(file_name, relative_path);
    close_db();
    printf_debug("main() return %d\n", EXIT_SUCCESS);
    return EXIT_SUCCESS;
}
