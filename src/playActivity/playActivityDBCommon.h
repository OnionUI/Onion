
#ifndef PLAY_ACTIVITY_COMMON_H
#define PLAY_ACTIVITY_COMMON_H

#define PLAY_ACTIVITY_DB_NEW_FILE "/mnt/SDCARD/Saves/CurrentProfile/play_activity/play_activity_db.sqlite"
#define ROMS_FOLDER "/mnt/SDCARD/Roms"
#define CMD_TO_RUN "/mnt/SDCARD/.tmp_update/cmd_to_run.sh"
#define ROM_NOT_FOUND -1

#define CLIENT_SOCK_FILE "play_activity.client.sock"
#define SERVER_SOCK_FILE "play_activity.server.sock"
#define BUF_SIZE 8192
#define PORT 25565

typedef struct rom rom;
typedef struct playactivity playactivity;
typedef struct PlayActivities PlayActivities;

struct ROM {
    int id;
    char *type;
    char *name;
    char *file_path;
    char *image_path;
};
struct PlayActivity {
    ROM *rom;
    int play_count;
    int play_time_total;
    int play_time_average;
    char *first_played_at;
    char *last_played_at;
};
struct PlayActivities {
    PlayActivity **play_activity;
    int count;
    int play_time_total;
};

#endif
