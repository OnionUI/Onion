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
#include "system/state.h"
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

#include "../playActivity/cacheDB.h"
#include "../playActivity/playActivityDB.h"
#include "./listMigration.h"

#define MAXHISTORY 100
#define MAXHROMNAMESIZE 250
#define MAXHROMPATHSIZE 150

#define ROM_SCREENS_DIR "/mnt/SDCARD/Saves/CurrentProfile/romScreens"

#define MAXFILENAMESIZE 250
#define MAXSYSPATHSIZE 80

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

static char sTotalTimePlayed[50] = "";

// Game history list
typedef struct {
    uint32_t hash;
    char name[MAXHROMNAMESIZE * 2];
    char shortname[STR_MAX * 2];
    char LaunchCommand[STR_MAX * 3 + 80];
    char totalTime[100];
    int gameIndex;
    int lineNumber;
    SDL_Surface *romScreen;
    char romScreenPath[STR_MAX * 4];
    char path[PATH_MAX * 2];
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

static bool __initial_romscreens_loaded = false;

void unloadRomScreen(int index)
{
    if (index < 0 || index >= game_list_len)
        return;
    Game_s *game = &game_list[index];

    if (game->romScreen != NULL) {
        SDL_FreeSurface(game->romScreen);
        game->romScreen = NULL;
    }
}

SDL_Surface *loadRomScreen(int index)
{
    if (index < 0 || index >= game_list_len)
        return NULL;

    Game_s *game = &game_list[index];

    if (game->romScreen == NULL) {
        char currPicture[STR_MAX * 2];
        sprintf(currPicture, ROM_SCREENS_DIR "/%" PRIu32 ".png", game->hash);
        // Show artwork
        if (!exists(currPicture))
            sprintf(currPicture, "%s/Imgs/%s.png", extractPath(game->path), file_removeExtension(game->name));

        // ports
        if (!exists(currPicture)) {
            sprintf(currPicture, "/mnt/SDCARD/Roms/PORTS/Imgs/%s.png", file_removeExtension(game->name));
            printf_debug("mario64 path %s\n", currPicture);
        }

        if (exists(currPicture)) {
            strcpy(game->romScreenPath, currPicture);
            game->romScreen = IMG_Load(currPicture);
        }
    }

    if (__initial_romscreens_loaded) {
        unloadRomScreen(index + 5);
        if (index > 5) {
            unloadRomScreen(index - 5);
        }
    }

    return game->romScreen;
}

void freeRomScreens()
{
    for (int i = 0; i < game_list_len; i++) {
        Game_s *game = &game_list[i];

        if (game->romScreen != NULL) {
            SDL_FreeSurface(game->romScreen);
            game->romScreen = NULL;
        }
    }
}

static void *_loadRomScreensThread(void *_)
{
    for (int i = 0; i < 10 && i < game_list_len; i++) {
        Game_s *game = &game_list[i];

        if (game->romScreen == NULL)
            loadRomScreen(i);
    }

    __initial_romscreens_loaded = true;

    return NULL;
}

void getGameName(char *name_out, const char *rom_path)
{
    CacheDBItem *cache_item = cache_db_find(rom_path);
    if (cache_item != NULL) {
        strcpy(name_out, cache_item->name);
        free(cache_item);
    }
    else {
        strcpy(name_out, file_removeExtension(basename(strdup(rom_path))));
    }
}

/**
 * @brief History extraction
 *
 */
void readHistory()
{
    FILE *file;
    char line[STR_MAX * 3];
    char *jsonContent;
    int nbGame = 0;
    int lineCounter = 0;
    file = fopen(getMiyooRecentFilePath(), "r");

    if (file == NULL) {
        print_debug("Error opening file");
        return;
    }

    while ((fgets(line, STR_MAX * 6, file) != NULL) && (nbGame < MAXHISTORY)) {
        char label[STR_MAX * 2];
        char rompath[STR_MAX * 2];
        char imgpath[STR_MAX * 2];
        char launch[STR_MAX * 2];
        int type;
        lineCounter++;

        jsonContent = (char *)malloc(strlen(line) + 1);
        if (jsonContent == NULL) {
            print_debug("Memory allocation error");
            fclose(file);
            return;
        }

        strcpy(jsonContent, line);
        printf_debug("jsonContent: %s\n", jsonContent);

        sscanf(strstr(jsonContent, "\"type\":") + 7, "%d", &type);

        if (type != 5)
            continue;

        print_debug("type 5");

        const char *labelStart = strstr(jsonContent, "\"label\":\"");
        if (labelStart != NULL) {
            labelStart += 9;
            const char *labelEnd = strchr(labelStart, '\"');
            strncpy(label, labelStart, labelEnd - labelStart);
            label[labelEnd - labelStart] = '\0';
        }
        printf_debug("label: %s\n", label);
        const char *rompathStart = strstr(jsonContent, "\"rompath\":\"");
        if (rompathStart != NULL) {
            rompathStart += 11;
            const char *rompathEnd = strchr(rompathStart, '\"');
            strncpy(rompath, rompathStart, rompathEnd - rompathStart);
            rompath[rompathEnd - rompathStart] = '\0';
        }
        printf_debug("rompath: %s\n", rompath);
        const char *imgpathStart = strstr(jsonContent, "\"imgpath\":\"");
        if (imgpathStart != NULL) {
            imgpathStart += 11;
            const char *imgpathEnd = strchr(imgpathStart, '\"');
            strncpy(imgpath, imgpathStart, imgpathEnd - imgpathStart);
            imgpath[imgpathEnd - imgpathStart] = '\0';
        }

        char *colonPosition = strchr(rompath, ':');
        if (colonPosition != NULL) {

            int position = (int)(colonPosition - rompath);

            char firstPart[position + 1];
            strncpy(firstPart, rompath, position);
            firstPart[position] = '\0';

            char secondPart[strlen(rompath) - position];
            strcpy(secondPart, colonPosition + 1);

            strcpy(launch, firstPart);
            strcpy(rompath, secondPart);
            printf_debug("launch cutted: %s\n", launch);
            printf_debug("rompath cutted: %s\n", rompath);
        }
        else {
            const char *launchStart = strstr(jsonContent, "\"launch\":\"");
            if (launchStart != NULL) {
                launchStart += 10;
                const char *launchEnd = strchr(launchStart, '\"');
                strncpy(launch, launchStart, launchEnd - launchStart);
                launch[launchEnd - launchStart] = '\0';
            }
        }

        printf_debug("launch: %s\n", launch);
        free(jsonContent);

        if (!exists(rompath) || !exists(launch))
            continue;

        Game_s *game = &game_list[nbGame];

        game->hash = FNV1A_Pippip_Yurii(rompath, strlen(rompath));

        game->lineNumber = lineCounter;
        game->romScreen = NULL;
        game->totalTime[0] = '\0';

        sprintf(game->LaunchCommand, "LD_PRELOAD=/mnt/SDCARD/miyoo/app/../lib/libpadsp.so \"%s\" \"%s\"", launch, rompath);

        getGameName(game->name, rompath);
        strcpy(game->path, rompath);
        file_cleanName(game->shortname, game->name);
        game->gameIndex = nbGame + 1;

        game = &game_list[nbGame];

        printf_debug("name: %s\n", game->name);
        printf_debug("shortname: %s\n", game->shortname);
        printf_debug("LaunchCommand: %s\n", game->LaunchCommand);
        printf_debug("totalTime: %i\n", game->totalTime);
        printf_debug("gameIndex: %i\n", game->gameIndex);
        printf_debug("lineNumber: %i\n", game->lineNumber);
        printf_debug("romScreenPath: %s\n", game->romScreenPath);
        printf_debug("path: %s\n", game->path);

        nbGame++;
    }

    game_list_len = nbGame;
    pthread_create(&thread_pt, NULL, _loadRomScreensThread, NULL);
}

void removeCurrentItem()
{
    Game_s *game = &game_list[current_game];

    printf_debug("removing: %s\n", game->name);
    printf_debug("linenumber: %i\n", game->lineNumber);

    if (game->romScreen != NULL) {
        SDL_FreeSurface(game->romScreen);
        game->romScreen = NULL;
    }

    file_delete_line(getMiyooRecentFilePath(), game->lineNumber);

    if (strlen(game->romScreenPath) > 0 && is_file(game->romScreenPath))
        remove(game->romScreenPath);

    // Copy next element value to current element
    for (int i = current_game; i < game_list_len - 1; i++) {
        game_list[i] = game_list[i + 1];
        game_list[i].lineNumber -= 1;
        game_list[i].gameIndex -= 1;
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

int main(int argc, char *argv[])
{
    log_setName("gameSwitcher");
    print_debug("\n\nDebug logging enabled");

    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    int b_force_migration = (argc > 1 && strcmp(argv[1], "force_migration") == 0);

    if (!is_file(RECENTLISTMIGRATED) || b_force_migration) {
        print_debug("Recent list migration started");
        migrateGameSwitcherList();
        char s_command[STR_MAX];
        sprintf(s_command, "touch %s", RECENTLISTMIGRATED);
        system(s_command);

        if (b_force_migration)
            return EXIT_SUCCESS;
    }

    SDL_InitDefault(true);

    SDL_BlitSurface(theme_background(), NULL, screen, NULL);
    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);

    readHistory();

    settings_load();
    lang_load();

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
    bool use_custom_header = theme_getImagePath(theme()->path, "extra/gs-top-bar", header_path);
    bool use_custom_footer = theme_getImagePath(theme()->path, "extra/gs-bottom-bar", footer_path);
    SDL_Surface *custom_header = use_custom_header ? IMG_Load(header_path) : NULL;
    SDL_Surface *custom_footer = use_custom_footer ? IMG_Load(footer_path) : NULL;

    int header_height = use_custom_header ? custom_header->h : 60;
    if (header_height == 1)
        header_height = 0;
    int footer_height = use_custom_footer ? custom_footer->h : 60;
    if (footer_height == 1)
        footer_height = 0;

    SDL_Surface *current_bg = NULL;
    // SDL_Rect frame = {theme()->frame.border_left, 0, 640 - theme()->frame.border_left - theme()->frame.border_right, 480};

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

            if (!changed && !brightness_changed && (surfaceGameName == NULL || surfaceGameName->w <= game_name_max_width))
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
                        int x_offset = (int)((640 - current_bg->w) / 2);
                        int y_offset = (int)((480 - current_bg->h) / 2);
                        SDL_Rect frame_bg = {x_offset, y_offset, 640, 480};
                        SDL_BlitSurface(current_bg, NULL, screen, &frame_bg);
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
                    if (strlen(game->totalTime) == 0) {
                        str_serializeTime(game->totalTime, play_activity_get_play_time(game->path));
                    }
                    strcpy(title_str, game->totalTime);

                    if (show_total) {
                        if (strlen(sTotalTimePlayed) == 0) {
                            str_serializeTime(sTotalTimePlayed, play_activity_get_total_play_time());
                        }
                        sprintf(title_str + strlen(title_str), " / %s", sTotalTimePlayed);
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

    if (exit_to_menu) {
        print_debug("Exiting to menu");
        remove("/mnt/SDCARD/.tmp_update/.runGameSwitcher");
        remove("/mnt/SDCARD/.tmp_update/cmd_to_run.sh");
    }

    else {
        print_debug("Resuming game");
        resumeGame(game_list[current_game].gameIndex);
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
