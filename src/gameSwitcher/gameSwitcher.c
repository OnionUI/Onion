#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <fcntl.h>
#include <libgen.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <pthread.h>
#include <signal.h>
#include <sqlite3/sqlite3.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "png/png.h"

#include "system/battery.h"
#include "system/keymap_sw.h"
#include "system/lang.h"
#include "system/settings.h"
#include "theme/background.h"
#include "theme/sound.h"
#include "theme/theme.h"
#include "utils/config.h"
#include "utils/file.h"
#include "utils/hash.h"
#include "utils/json.h"
#include "utils/keystate.h"
#include "utils/log.h"
#include "utils/msleep.h"
#include "utils/sdl_init.h"
#include "utils/str.h"
#include "utils/surfaceSetAlpha.h"

#include "../playActivity/playActivityDB.h"

#define MAXHISTORY 100
#define MAXHROMNAMESIZE 250
#define MAXHROMPATHSIZE 150

#define ROM_SCREENS_DIR "/mnt/SDCARD/Saves/CurrentProfile/romScreens"
#define HISTORY_PATH \
    "/mnt/SDCARD/Saves/CurrentProfile/lists/content_history.lpl"

#define MAXFILENAMESIZE 250
#define MAXSYSPATHSIZE 80

#define MAXHRACOMMAND 500
#define LOWBATRUMBLE 10

// Max number of records in the DB
#define MAXVALUES 1000

#define GPIO_DIR1 "/sys/class/gpio/"
#define GPIO_DIR2 "/sys/devices/gpiochip0/gpio/"

#define VIEW_NORMAL 0
#define VIEW_MINIMAL 1
#define VIEW_FULLSCREEN -1

static bool quit = false;
static bool exit_to_menu = false;

static pthread_t thread_pt;

static void sigHandler(int sig)
{
    switch (sig) {
    case SIGINT:
    case SIGTERM:
        exit_to_menu = true;
        quit = true;
        break;
    default:
        break;
    }
}

char sTotalTimePlayed[50];

static PlayActivities *play_activities;

// Game history list
typedef struct {
    uint32_t hash;
    char name[MAXHROMNAMESIZE];
    char shortname[STR_MAX];
    char core[STR_MAX];
    char RACommand[STR_MAX * 2 + 80];
    char totalTime[100];
    int jsonIndex;
    int is_duplicate;
    SDL_Surface *romScreen;
    char romScreenPath[STR_MAX * 2];
} Game_s;
static Game_s game_list[MAXHISTORY];

static int game_list_len = 0;
static int current_game = 0;

static SDL_Surface *surfaceGameName = NULL;
static int gameNameScrollX = 0;
static int gameNameScrollSpeed = 10;
static int gameNameScrollStart = 20;
static int gameNameScrollEnd = 20;

static cJSON *json_root = NULL;
static cJSON *json_items = NULL;

void removeParentheses(char *str_out, const char *str_in)
{
    char temp[STR_MAX];
    int len = strlen(str_in);
    int c = 0;
    bool inside = false;
    char end_char;

    for (int i = 0; i < len && i < STR_MAX; i++) {
        if (!inside && (str_in[i] == '(' || str_in[i] == '[')) {
            end_char = str_in[i] == '(' ? ')' : ']';
            inside = true;
            continue;
        }
        else if (inside) {
            if (str_in[i] == end_char)
                inside = false;
            continue;
        }
        temp[c++] = str_in[i];
    }

    temp[c] = '\0';

    str_trim(str_out, STR_MAX - 1, temp, false);
}

SDL_Surface *loadRomScreen(int index)
{
    if (index < 0 || index >= game_list_len)
        return NULL;

    Game_s *game = &game_list[index];

    if (game->romScreen == NULL) {
        char currPicture[STR_MAX * 2];
        sprintf(currPicture, ROM_SCREENS_DIR "/%" PRIu32 "_%s.png", game->hash,
                game->core);

        if (!exists(currPicture))
            sprintf(currPicture, ROM_SCREENS_DIR "/%" PRIu32 ".png",
                    game->hash);

        if (!exists(currPicture))
            sprintf(currPicture, ROM_SCREENS_DIR "/%s.png",
                    file_removeExtension(game->name));

        if (exists(currPicture)) {
            game->romScreen = IMG_Load(currPicture);
            strcpy(game->romScreenPath, currPicture);
        }
    }

    return game->romScreen;
}

void freeRomScreens()
{
    for (int i = 0; i < game_list_len; i++) {
        Game_s *game = &game_list[i];

        if (game->romScreen != NULL)
            SDL_FreeSurface(game->romScreen);
    }
}

static void *_loadRomScreensThread(void *_)
{
    for (int i = 0; i < game_list_len; i++) {
        Game_s *game = &game_list[i];

        if (game->romScreen == NULL)
            loadRomScreen(i);
    }

    return NULL;
}

bool getGameNameFromCache(const char *cache_path, const char *rom_path,
                          char *name_out)
{
    sqlite3 *db;
    sqlite3_stmt *res;

    int rc = sqlite3_open(cache_path, &db);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return false;
    }

    char cache_path_clone[STR_MAX];
    strcpy(cache_path_clone, cache_path);

    char rom_path_clone[STR_MAX];
    strcpy(rom_path_clone, rom_path);

    char *rom_path_slice = strstr(rom_path_clone, "Roms");
    if (rom_path_slice == NULL)
        return false;

    const char *sql =
        sqlite3_mprintf("SELECT pinyin FROM '%q_roms' WHERE path LIKE '%%%q'",
                        basename(dirname(cache_path_clone)), rom_path_slice);
    printf_debug("query: %s\n", sql);

    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return false;
    }

    rc = sqlite3_step(res);

    if (rc == SQLITE_ROW)
        strcpy(name_out, (const char *)sqlite3_column_text(res, 0));

    sqlite3_finalize(res);
    sqlite3_close(db);

    return rc == SQLITE_ROW;
}

void moveToParentDir(char *file_path)
{
    char *c = &file_path[strlen(file_path)];

    while (c > file_path && *c != '/')
        c--;

    if (*c == '/')
        *c = '\0';

    return;
}

void getGameName(const char *rom_path, char *name_out)
{
    char rom_path_clone[STR_MAX];
    strcpy(rom_path_clone, rom_path);

    char *base_name = basename(rom_path_clone);
    char *dir_path = dirname(rom_path_clone);

    char name_path[STR_MAX];
    sprintf(name_path, "%s/.game_config/%s.name", dir_path, base_name);

    if (is_file(name_path)) {
        FILE *fp;
        file_get(fp, name_path, "%[^\n]", name_out);
        return;
    }

    char cache_dir[STR_MAX];
    strcpy(cache_dir, dir_path);

    char cache_path[STR_MAX * 2];

    do {
        snprintf(cache_path, STR_MAX * 2 - 1, "%s/%s_cache6.db", cache_dir,
                 basename(cache_dir));

        moveToParentDir(cache_dir);

        if (!is_file(cache_path))
            continue;

        if (getGameNameFromCache(cache_path, rom_path, name_out))
            return;
    } while (strcmp("/mnt/SDCARD/Roms", cache_dir) > 0 &&
             strlen(cache_dir) > 16);

    strcpy(name_out, file_removeExtension(base_name));
}

void serializeTime(int nTime, char *dest_str)
{
    if (nTime >= 0) {
        int h = nTime / 3600;
        int m = (nTime - 3600 * h) / 60;
        if (h > 0)
            sprintf(dest_str, "%dh %dm", h, m);
        else
            sprintf(dest_str, "%dm", m);
    }
    else {
        strcpy(dest_str, "0m");
    }
}

/**
 * @brief History extraction
 *
 */
void readHistory()
{
    game_list_len = 0;

    if (!exists(HISTORY_PATH)) {
        print_debug("History file missing");
        return;
    }

    char rom_path[STR_MAX], core_path[STR_MAX];

    if (json_items == NULL) {
        json_root = json_load(HISTORY_PATH);
        json_items = cJSON_GetObjectItem(json_root, "items");
    }

    for (int nbGame = 0; nbGame < MAXHISTORY; nbGame++) {
        cJSON *subitem = cJSON_GetArrayItem(json_items, nbGame);

        if (subitem == NULL)
            break;

        if (!json_getString(subitem, "path", rom_path) ||
            !json_getString(subitem, "core_path", core_path))
            continue;

        if (strncmp("/mnt/SDCARD/App", rom_path, 15) == 0)
            continue;

        if (!exists(core_path) || !exists(rom_path))
            continue;

        Game_s *game = &game_list[game_list_len];

        sprintf(game->RACommand,
                "LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so ./retroarch -v "
                "-L \"%s\" \"%s\"",
                core_path, rom_path);

        getGameName(rom_path, game->name);

        strcpy(game->core, basename(core_path));
        str_split(game->core, "_libretro");

        game->hash = FNV1A_Pippip_Yurii(rom_path, strlen(rom_path));
        game->jsonIndex = nbGame;
        game->romScreen = NULL;
        game->is_duplicate = 0;

        char shortname[STR_MAX];
        removeParentheses(shortname, game->name);
        strcpy(game->shortname, shortname);

        char rom_path_clone[STR_MAX];
        strcpy(rom_path_clone, rom_path);

        int nTime = 0;
        char *rom_path_slice = strstr(rom_path_clone, "Roms");
        if (rom_path_slice != NULL) {
            ROM *romObject = rom_find_by_file_path(rom_path_slice);
            for (int i = 0; i < play_activities->count; i++) {
                PlayActivity *playActivityObject = play_activities->play_activity[i];
                if (romObject->id == playActivityObject->rom->id) {
                    nTime = playActivityObject->play_time_total;
                }
            }
        }

        serializeTime(nTime, game->totalTime);

        printf_debug(
            "Game loaded:\n\tname: '%s' (%s)\n\tcmd: '%s'\n\thash: %" PRIu32
            "\n\tidx: %d\n\ttime: %s\n\n",
            game->name, game->shortname, game->RACommand, game->hash,
            game->jsonIndex, game->totalTime);

        // Check for duplicates
        for (int i = 0; i < game_list_len; i++) {
            Game_s *other = &game_list[i];
            if (other->hash == game->hash) {
                other->is_duplicate += 1;
                game->is_duplicate = other->is_duplicate;
            }
        }

        game_list_len++;
    }

    pthread_create(&thread_pt, NULL, _loadRomScreensThread, NULL);
}

void removeCurrentItem()
{
    Game_s *game = &game_list[current_game];

    printf_debug("removing: %s\n", game->name);

    if (game->romScreen != NULL)
        SDL_FreeSurface(game->romScreen);

    if (game->is_duplicate > 0) {
        // Check for duplicates
        for (int i = 0; i < game_list_len; i++) {
            Game_s *other = &game_list[i];
            if (other->hash == game->hash)
                other->is_duplicate -= 1;
        }
    }

    if (json_items != NULL)
        cJSON_DeleteItemFromArray(json_items, game->jsonIndex);

    json_save(json_root, HISTORY_PATH);

    if (strlen(game->romScreenPath) > 0 && is_file(game->romScreenPath))
        remove(game->romScreenPath);

    // Copy next element value to current element
    for (int i = current_game; i < game_list_len - 1; i++) {
        game_list[i] = game_list[i + 1];
        game_list[i].jsonIndex -= 1;
    }

    game_list_len--;
}

int checkQuitAction(void)
{
    FILE *fp;
    char prev_state[10];
    file_get(fp, "/tmp/prev_state", "%s", prev_state);
    if (strncmp(prev_state, "mainui", 6) == 0)
        return 1;
    return 0;
}

int main(void)
{
    print_debug("Debug logging enabled");

    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    play_activities = play_activity_find_all();
    serializeTime(play_activities->play_time_total, sTotalTimePlayed);

    readHistory();
    print_debug("Read ROM DB and history");

    settings_load();
    lang_load();

    SDL_InitDefault(true);

    SDL_Color color_white = {255, 255, 255};
    SDL_Surface *transparent_bg = SDL_CreateRGBSurface(
        0, 640, 480, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    SDL_FillRect(transparent_bg, NULL, 0xBE000000);

    SDL_Surface *arrow_left = resource_getSurface(LEFT_ARROW_WB);
    SDL_Surface *arrow_right = resource_getSurface(RIGHT_ARROW_WB);
    int game_name_padding = arrow_left->w + 20;
    int game_name_max_width = 640 - 2 * game_name_padding;
    SDL_Rect game_name_size = {0, 0};

    int battery_percentage = battery_getPercentage();

    bool changed = true;
    bool current_game_changed = true;
    bool brightness_changed = false;

    KeyState keystate[320] = {(KeyState)0};
    bool menu_pressed = false;
    bool combo_key = false;
    bool select_pressed = false;
    bool select_combo_key = false;

    mkdirs("/mnt/SDCARD/.tmp_update/config/gameSwitcher");

    bool view_min = config_flag_get("gameSwitcher/minimal");
    bool show_time = config_flag_get("gameSwitcher/showTime");
    bool show_total = !config_flag_get("gameSwitcher/hideTotal");
    bool show_legend = !config_flag_get("gameSwitcher/hideLegend");
    int view_mode = view_min ? VIEW_MINIMAL : VIEW_NORMAL, view_restore;

    SDLKey changed_key = SDLK_UNKNOWN;
    int button_y_repeat = 0;

    uint32_t acc_ticks = 0, last_ticks = SDL_GetTicks(), time_step = 1000 / 30;

    uint32_t legend_start = last_ticks;
    uint32_t legend_timeout = 5000;

    uint32_t brightness_start = last_ticks;
    uint32_t brightness_timeout = 2000;

    char header_path[STR_MAX], footer_path[STR_MAX];
    bool use_custom_header =
        theme_getImagePath(theme()->path, "extra/gs-top-bar", header_path);
    bool use_custom_footer =
        theme_getImagePath(theme()->path, "extra/gs-bottom-bar", footer_path);
    SDL_Surface *custom_header =
        use_custom_header ? IMG_Load(header_path) : NULL;
    SDL_Surface *custom_footer =
        use_custom_footer ? IMG_Load(footer_path) : NULL;

    int header_height = use_custom_header ? custom_header->h : 60;
    if (header_height == 1)
        header_height = 0;
    int footer_height = use_custom_footer ? custom_footer->h : 60;
    if (footer_height == 1)
        footer_height = 0;

    SDL_Surface *current_bg = NULL;
    SDL_Rect frame = {
        theme()->frame.border_left, 0,
        640 - theme()->frame.border_left - theme()->frame.border_right, 480};

    while (!quit) {
        uint32_t ticks = SDL_GetTicks();
        acc_ticks += ticks - last_ticks;
        last_ticks = ticks;

        if (show_legend && ticks - legend_start > legend_timeout) {
            show_legend = false;
            config_flag_set("gameSwitcher/hideLegend", true);
            changed = true;
        }

        if (brightness_changed &&
            ticks - brightness_start > brightness_timeout) {
            brightness_changed = false;
            changed = true;
        }

        if (updateKeystate(keystate, &quit, true, &changed_key)) {
            if (menu_pressed && changed_key != SW_BTN_MENU)
                combo_key = true;
            if (select_pressed && changed_key != SW_BTN_SELECT)
                select_combo_key = true;

            if (keystate[SW_BTN_MENU] == PRESSED)
                menu_pressed = true;

            if (menu_pressed && keystate[SW_BTN_MENU] == RELEASED) {
                if (!combo_key) {
                    quit = true;
                    break;
                }
                menu_pressed = false;
                combo_key = false;
            }

            if (keystate[SW_BTN_RIGHT] >= PRESSED) {
                if (current_game < game_list_len - 1) {
                    current_game++;
                    current_game_changed = true;
                    changed = true;
                }
            }

            if (keystate[SW_BTN_LEFT] >= PRESSED) {
                if (current_game > 0) {
                    current_game--;
                    current_game_changed = true;
                    changed = true;
                }
            }

            if (keystate[SW_BTN_START] == PRESSED) {
                exit_to_menu = true;
                break;
            }

            if (keystate[SW_BTN_A] == PRESSED)
                break;

            if (keystate[SW_BTN_B] == PRESSED) {
                exit_to_menu = true;
                quit = true;
                break;
            }

            if (keystate[changed_key] == PRESSED && changed_key != SW_BTN_UP &&
                changed_key != SW_BTN_DOWN)
                brightness_changed = false;

            if (keystate[SW_BTN_UP] >= PRESSED) {
                // Change brightness
                if (settings.brightness < 10) {
                    settings_setBrightness(settings.brightness + 1, true, true);
                }
                brightness_changed = true;
                brightness_start = last_ticks;
                changed = true;
            }

            if (keystate[SW_BTN_DOWN] >= PRESSED) {
                // Change brightness
                if (settings.brightness > 0) {
                    settings_setBrightness(settings.brightness - 1, true, true);
                }
                brightness_changed = true;
                brightness_start = last_ticks;
                changed = true;
            }

            if (combo_key ||
                (select_pressed && ((changed_key == SW_BTN_L2 &&
                                     keystate[SW_BTN_L2] == RELEASED) ||
                                    (changed_key == SW_BTN_R2 &&
                                     keystate[SW_BTN_R2] == RELEASED)))) {
                settings_load();
                brightness_changed = false;
                changed = true;
            }

            if (changed_key == SW_BTN_SELECT) {
                if (keystate[SW_BTN_SELECT] == PRESSED)
                    select_pressed = true;
                if (keystate[SW_BTN_SELECT] == RELEASED) {
                    if (!select_combo_key) {
                        show_legend = true;
                        legend_start = last_ticks;

                        if (!show_time && !show_total)
                            show_time = true, show_total = false;
                        else if (show_time && !show_total)
                            show_time = true, show_total = true;
                        else
                            show_time = false, show_total = false;

                        config_flag_set("gameSwitcher/showTime", show_time);
                        config_flag_set("gameSwitcher/hideTotal", !show_total);

                        changed = true;
                    }
                    select_pressed = false;
                    select_combo_key = false;
                }
            }

            if (changed_key == SW_BTN_Y && keystate[SW_BTN_Y] == RELEASED) {
                if (button_y_repeat < 75) {
                    view_mode = view_mode == VIEW_FULLSCREEN ? view_restore
                                                             : !view_mode;
                    config_flag_set("gameSwitcher/minimal",
                                    view_mode == VIEW_MINIMAL);
                    changed = true;
                }
                button_y_repeat = 0;
            }

            if (keystate[SW_BTN_X] == PRESSED) {
                if (game_list_len != 0) {
                    theme_renderDialog(
                        screen, "Remove from history",
                        "Are you sure you want to\nremove game from history?",
                        true);
                    SDL_BlitSurface(screen, NULL, video, NULL);
                    SDL_Flip(video);
                    sound_change();

                    while (!quit) {
                        if (updateKeystate(keystate, &quit, true, NULL)) {
                            if (keystate[SW_BTN_A] == PRESSED) {
                                removeCurrentItem();
                                if (current_game > 0)
                                    current_game--;
                                current_game_changed = true;
                                loadRomScreen(current_game);
                                changed = true;
                                break;
                            }
                            if (keystate[SW_BTN_B] == PRESSED) {
                                changed = true;
                                break;
                            }
                        }
                    }
                }
            }

            if (changed)
                sound_change();
        }

        if (keystate[SW_BTN_Y] == PRESSED && view_mode != VIEW_FULLSCREEN) {
            button_y_repeat++;
            if (button_y_repeat >= 75) {
                view_restore = view_mode;
                view_mode = VIEW_FULLSCREEN;
                changed = true;
            }
        }

        if (battery_hasChanged(ticks, &battery_percentage))
            changed = true;

        if (acc_ticks >= time_step) {
            acc_ticks -= time_step;

            if (!changed && !brightness_changed &&
                (surfaceGameName == NULL ||
                 surfaceGameName->w <= game_name_max_width))
                continue;

            if (changed) {
                SDL_BlitSurface(theme_background(), NULL, screen, NULL);

                if (game_list_len == 0) {
                    current_bg = NULL;
                    SDL_Surface *empty = resource_getSurface(EMPTY_BG);
                    SDL_Rect empty_rect = {320 - empty->w / 2,
                                           240 - empty->h / 2};
                    SDL_BlitSurface(empty, NULL, screen, &empty_rect);
                }
                else {
                    current_bg = loadRomScreen(current_game);

                    if (current_bg != NULL) {
                        if (view_mode == VIEW_NORMAL)
                            SDL_BlitSurface(current_bg, &frame, screen, &frame);
                        else
                            SDL_BlitSurface(current_bg, NULL, screen, NULL);
                    }
                }
            }

            Game_s *game = &game_list[current_game];

            if (view_mode != VIEW_FULLSCREEN && game_list_len > 0) {
                SDL_Rect game_name_bg_size = {0, 0, 640, 60};
                SDL_Rect game_name_bg_pos = {0, 360};

                if (view_mode == VIEW_NORMAL) {
                    game_name_bg_size.x = game_name_bg_pos.x =
                        theme()->frame.border_left;
                    game_name_bg_size.w -= theme()->frame.border_left +
                                           theme()->frame.border_right;
                }

                game_name_bg_size.y = game_name_bg_pos.y =
                    view_mode == VIEW_NORMAL ? (480 - footer_height - 60) : 420;

                SDL_BlitSurface(current_bg, &game_name_bg_size, screen,
                                &game_name_bg_pos);
                SDL_BlitSurface(transparent_bg, &game_name_bg_size, screen,
                                &game_name_bg_pos);

                if (current_game > 0) {
                    SDL_Rect arrow_left_rect = {theme()->frame.border_left + 10,
                                                game_name_bg_pos.y + 30 -
                                                    arrow_left->h / 2};
                    SDL_BlitSurface(arrow_left, NULL, screen, &arrow_left_rect);
                }

                if (current_game < game_list_len - 1) {
                    SDL_Rect arrow_right_rect = {
                        630 - theme()->frame.border_right - arrow_right->w,
                        game_name_bg_pos.y + 30 - arrow_right->h / 2};
                    SDL_BlitSurface(arrow_right, NULL, screen,
                                    &arrow_right_rect);
                }

                char game_name_str[STR_MAX * 2 + 4];

                if (game->is_duplicate > 0)
                    snprintf(game_name_str, STR_MAX * 2 + 3, "%s (%s)",
                             game->shortname, game->core);
                else
                    strcpy(game_name_str, game->shortname);

                if (current_game_changed) {
                    if (surfaceGameName != NULL)
                        SDL_FreeSurface(surfaceGameName);
                    surfaceGameName = TTF_RenderUTF8_Blended(
                        resource_getFont(TITLE), game_name_str, color_white);
                    game_name_size.w = surfaceGameName->w < game_name_max_width
                                           ? surfaceGameName->w
                                           : game_name_max_width;
                    game_name_size.h = surfaceGameName->h;
                    gameNameScrollX =
                        -gameNameScrollStart * gameNameScrollSpeed;
                }

                SDL_Rect game_name_rect = {320 - surfaceGameName->w / 2,
                                           game_name_bg_pos.y + 30 -
                                               surfaceGameName->h / 2};
                if (game_name_rect.x < game_name_padding)
                    game_name_rect.x = game_name_padding;

                game_name_size.x =
                    gameNameScrollX < (surfaceGameName->w - game_name_size.w)
                        ? (gameNameScrollX > 0 ? gameNameScrollX : 0)
                        : surfaceGameName->w - game_name_size.w;

                SDL_BlitSurface(surfaceGameName, &game_name_size, screen,
                                &game_name_rect);

                if (surfaceGameName->w > game_name_max_width) {
                    gameNameScrollX += gameNameScrollSpeed;

                    if (gameNameScrollX >
                        (surfaceGameName->w - game_name_size.w +
                         gameNameScrollEnd * gameNameScrollSpeed))
                        gameNameScrollX =
                            -gameNameScrollStart * gameNameScrollSpeed;
                }
            }

            if (!changed) {
                SDL_BlitSurface(screen, NULL, video, NULL);
                SDL_Flip(video);
                continue;
            }

            if (view_mode == VIEW_NORMAL) {
                if (custom_footer) {
                    if (footer_height > 0) {
                        SDL_Rect footer_rect = {0, 480 - custom_footer->h};
                        SDL_BlitSurface(custom_footer, NULL, screen,
                                        &footer_rect);
                    }
                }
                else {
                    theme_renderFooter(screen);
                    theme_renderStandardHint(
                        screen, lang_get(LANG_RESUME, LANG_FALLBACK_RESUME),
                        lang_get(LANG_BACK, LANG_FALLBACK_BACK));
                    theme_renderFooterStatus(
                        screen, game_list_len > 0 ? current_game + 1 : 0,
                        game_list_len);
                }
            }

            if (view_mode == VIEW_NORMAL) {
                char title_str[STR_MAX] = "GameSwitcher";
                if (show_time && game_list_len > 0) {
                    strcpy(title_str, game->totalTime);

                    if (show_total) {
                        sprintf(title_str + strlen(title_str), " / %s",
                                sTotalTimePlayed);
                    }
                }

                if (custom_header) {
                    if (header_height > 0) {
                        SDL_BlitSurface(custom_header, NULL, screen, NULL);
                        SDL_Surface *title = TTF_RenderUTF8_Blended(
                            resource_getFont(TITLE), title_str,
                            theme()->title.color);
                        if (title) {
                            SDL_Rect title_rect = {320 - title->w / 2,
                                                   (header_height - title->h) /
                                                       2};
                            SDL_BlitSurface(title, NULL, screen, &title_rect);
                            SDL_FreeSurface(title);
                        }
                        theme_renderHeaderBatteryCustom(
                            screen, battery_percentage, header_height);
                    }
                }
                else {
                    theme_renderHeader(screen, title_str, false);
                    theme_renderHeaderBattery(screen, battery_percentage);
                }
            }

            if (show_legend && view_mode != VIEW_FULLSCREEN) {
                SDL_Surface *legend = resource_getSurface(LEGEND_GAMESWITCHER);
                SDL_Rect legend_rect = {640 - legend->w,
                                        view_mode == VIEW_NORMAL ? header_height
                                                                 : 0};
                SDL_BlitSurface(legend, NULL, screen, &legend_rect);
            }

            if (brightness_changed) {
                // Display luminosity slider
                SDL_Surface *brightness =
                    resource_getBrightness(settings.brightness);
                bool vertical = brightness->h > brightness->w;
                SDL_Rect brightness_rect = {
                    0,
                    (view_mode == VIEW_NORMAL ? 240 : 210) - brightness->h / 2};
                if (!vertical) {
                    brightness_rect.x = 320 - brightness->w / 2;
                    brightness_rect.y =
                        view_mode == VIEW_NORMAL ? header_height : 0;
                }
                SDL_BlitSurface(brightness, NULL, screen, &brightness_rect);
            }

            SDL_BlitSurface(screen, NULL, video, NULL);
            SDL_Flip(video);

            changed = false;
            current_game_changed = false;
        }
    }

    screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);

    remove("/mnt/SDCARD/.tmp_update/.runGameSwitcher");
    remove("/mnt/SDCARD/.tmp_update/cmd_to_run.sh");

    if (exit_to_menu)
        print_debug("Exiting to menu");
    else {
        print_debug("Resuming game");
        FILE *file = fopen("/mnt/SDCARD/.tmp_update/cmd_to_run.sh", "w");
        fputs(game_list[current_game].RACommand, file);
        fclose(file);
    }

#ifndef PLATFORM_MIYOOMINI
    msleep(200);
#endif

    if (json_root != NULL)
        cJSON_free(json_root);

    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);

    if (custom_header != NULL)
        SDL_FreeSurface(custom_header);
    if (custom_footer != NULL)
        SDL_FreeSurface(custom_footer);
    if (surfaceGameName != NULL)
        SDL_FreeSurface(surfaceGameName);

    resources_free();
    SDL_FreeSurface(transparent_bg);

    freeRomScreens();

    SDL_FreeSurface(screen);
    SDL_FreeSurface(video);

    TTF_Quit();
    SDL_Quit();

    return EXIT_SUCCESS;
}
