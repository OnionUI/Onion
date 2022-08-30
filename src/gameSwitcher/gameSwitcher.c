#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <fcntl.h>
#include <stdbool.h>
#include <libgen.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#include "png/png.h"

#include "utils/utils.h"
#include "utils/json.h"
#include "utils/hash.h"
#include "utils/msleep.h"
#include "utils/log.h"
#include "utils/keystate.h"
#include "utils/config.h"
#include "utils/surfaceSetAlpha.h"
#include "utils/sdl_init.h"
#include "utils/imageCache.h"
#include "system/battery.h"
#include "system/keymap_sw.h"
#include "system/settings.h"
#include "system/lang.h"
#include "theme/theme.h"
#include "theme/background.h"
#include "theme/sound.h"

#define MAXHISTORY 50
#define MAXHROMNAMESIZE 250
#define MAXHROMPATHSIZE 150

#define ROM_SCREENS_DIR "/mnt/SDCARD/Saves/CurrentProfile/romScreens"
#define ROM_DB_PATH "/mnt/SDCARD/Saves/CurrentProfile/saves/playActivity.db"
#define HISTORY_PATH "/mnt/SDCARD/Saves/CurrentProfile/lists/content_history.lpl"
#define SYSTEM_CONFIG "/appconfigs/system.json"

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
static void sigHandler(int sig)
{
    switch (sig) {
        case SIGINT:
        case SIGTERM:
            quit = true;
            break;
        default: break;
    }
}

char sTotalTimePlayed[50];

// Game history list
typedef struct
{
    uint32_t hash;
    char name[MAXHROMNAMESIZE];
    char shortname[STR_MAX];
    char RACommand[STR_MAX * 2 + 80] ;
    char totalTime[100];
    int jsonIndex;
} Game_s;
static Game_s game_list[MAXHISTORY];

static int game_list_len = 0;
static int current_game = 0;

static cJSON* json_root = NULL;
static cJSON* items = NULL;

// Play activity database
struct structPlayActivity
{
    char name[100];
    int playTime;
}
rom_list[MAXVALUES];
int rom_list_len = 0;
int bDisplayBoxArt = 0;

int readRomDB()
{
    int totalTimePlayed = 0 ;
    // Check to avoid corruption
    if (exists(ROM_DB_PATH)) {
        FILE * file = fopen(ROM_DB_PATH, "rb");
        fread(rom_list, sizeof(rom_list), 1, file);
        rom_list_len = 0;

        for (int i=0; i<MAXVALUES; i++){
            if (strlen(rom_list[i].name) == 0)
                break;
            totalTimePlayed += rom_list[rom_list_len].playTime;
            rom_list_len++;
        }

        int h = totalTimePlayed / 3600;
        int m = (totalTimePlayed - 3600 * h) / 60;
        if (h > 0)
            snprintf(sTotalTimePlayed, sizeof(sTotalTimePlayed) - 1, "%dh %dm", h, m);
        else
            snprintf(sTotalTimePlayed, sizeof(sTotalTimePlayed) - 1, "%dm", m);
        fclose(file);
    }
    return 1;
}

int searchRomDB(char* romName){
    int position = -1;

    for (int i = 0 ; i < rom_list_len ; i++) {
        if (strcmp(rom_list[i].name,romName) == 0){
            position = i;
            break;
        }
    }
    return position;
}

void removeParentheses(char *str_out, const char *str_in)
{
    char temp[STR_MAX];
    int len = strlen(str_in);
    int c = 0;
    bool inside = false;

    for (int i = 0; i < len && i < STR_MAX; i++) {
        if (!inside && str_in[i] == '(') {
            inside = true;
            continue;
        }
        else if (inside) {
            if (str_in[i] == ')') inside = false;
            continue;
        }
        temp[c++] = str_in[i];
    }

    temp[c] = '\0';

    str_trim(str_out, STR_MAX - 1, temp, false);
}

void readHistory()
{
    // History extraction
    game_list_len = 0;

    if (!exists(HISTORY_PATH))
        return;

    char path[STR_MAX], core_path[STR_MAX];

    json_root = json_load(HISTORY_PATH);
    items = cJSON_GetObjectItem(json_root, "items");

    for (int nbGame = 0; nbGame < MAXHISTORY; nbGame++) {
        cJSON *subitem = cJSON_GetArrayItem(items, nbGame);

        if (subitem == NULL)
            continue;

        if (!json_getString(subitem, "path", path) || !json_getString(subitem, "core_path", core_path))
            continue;

        if (!exists(core_path) || !exists(path))
            continue;

        Game_s *game = &game_list[game_list_len];
        sprintf(game->RACommand, "LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so ./retroarch -v -L \"%s\" \"%s\"", core_path, path);
        strcpy(game->name, basename(path));
        game->hash = FNV1A_Pippip_Yurii(path, strlen(path));
        game->jsonIndex = nbGame;

        // Rom name
        int nTimePosition = searchRomDB(game->name);
        int nTime = nTimePosition >= 0 ? rom_list[nTimePosition].playTime : 0;
        if (nTime >= 0) {
            int h = nTime / 3600;
            int m = (nTime - 3600 * h) / 60;
            if (h > 0)
                snprintf(game->totalTime, sizeof(game->totalTime) - 1, "%dh %dm", h, m);
            else
                snprintf(game->totalTime, sizeof(game->totalTime) - 1, "%dm", m);
        }

        game_list_len++;
    }

    for (int i = 0; i < game_list_len; i++) {
        Game_s *game = &game_list[i];
        char shortname[STR_MAX];
        removeParentheses(shortname, file_removeExtension(game->name));
        strcpy(game->shortname, shortname);
    }
}

void removeCurrentItem() {
    imageCache_removeItem(current_game);
    if (items != NULL)
        cJSON_DeleteItemFromArray(items, game_list[current_game].jsonIndex);
    json_save(json_root, HISTORY_PATH);
}

SDL_Surface* loadRomscreen(int index)
{
    Game_s *game = &game_list[index];
    char currPicture[STR_MAX];
    sprintf(currPicture, ROM_SCREENS_DIR "/%"PRIu32".png", game->hash);
    if (!exists(currPicture))
        sprintf(currPicture, ROM_SCREENS_DIR "/%s.png", file_removeExtension(game->name));
    if (exists(currPicture))
        return IMG_Load(currPicture);
    print_debug("Rom screen failed");
    return NULL;
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

    readRomDB();
    readHistory();

    print_debug("Read ROM DB and history");

    imageCache_load(&current_game, loadRomscreen, game_list_len);

	settings_load();
	lang_load();

    SDL_Color color_white = {255, 255, 255};

    int nExitToMiyoo = 0;

    SDL_InitDefault(true);

    print_debug("Loading images");
    
    SDL_Surface *transparent_bg = SDL_CreateRGBSurface(0, 640, 480, 32,
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    SDL_FillRect(transparent_bg, NULL, 0xBE000000);

    SDL_Surface *arrow_left = resource_getSurface(LEFT_ARROW_WB);
    SDL_Surface *arrow_right = resource_getSurface(RIGHT_ARROW_WB);
    int game_name_padding = arrow_left->w + 20;
    int game_name_max_width = 640 - 2 * game_name_padding;
    SDL_Rect game_name_size = {0, 0};

    int battery_percentage = battery_getPercentage();

    bool changed = true;
    bool brightness_changed = false;
    bool image_drawn = false;

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

	uint32_t acc_ticks = 0,
			 last_ticks = SDL_GetTicks(),
			 time_step = 1000 / 30;

    uint32_t start = last_ticks;
    uint32_t legend_timeout = 5000;

    char header_path[STR_MAX], footer_path[STR_MAX];
    bool use_custom_header = theme_getImagePath(theme()->path, "extra/gs-top-bar", header_path);
    bool use_custom_footer = theme_getImagePath(theme()->path, "extra/gs-bottom-bar", footer_path);
    SDL_Surface *custom_header = use_custom_header ? IMG_Load(header_path) : NULL;
    SDL_Surface *custom_footer = use_custom_footer ? IMG_Load(footer_path) : NULL;

    int header_height = use_custom_header ? custom_header->h : 60;
    if (header_height == 1) header_height = 0;
    int footer_height = use_custom_footer ? custom_footer->h : 60;
    if (footer_height == 1) footer_height = 0;

    SDL_Rect frame = {theme()->frame.border_left, 0, 640 - theme()->frame.border_right, 480};

	while (!quit) {
		uint32_t ticks = SDL_GetTicks();
		acc_ticks += ticks - last_ticks;
		last_ticks = ticks;

        brightness_changed = false;

        if (show_legend && ticks - start > legend_timeout) {
            show_legend = false;
            config_flag_set("gameSwitcher/hideLegend", true);
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
                    changed = true;
                }
            }

            if (keystate[SW_BTN_LEFT] >= PRESSED) {
                if (current_game > 0) {
                    current_game--;
                    changed = true;
                }
            }

            if (keystate[SW_BTN_START] == PRESSED) {
                nExitToMiyoo = 1;
                break;
            }

            if (keystate[SW_BTN_A] == PRESSED)
                break;
            
            if (keystate[SW_BTN_B] == PRESSED) {
                nExitToMiyoo = checkQuitAction();
                quit = true;
                break;
            }

            if (keystate[SW_BTN_UP] >= PRESSED){
                // Change brightness
                if (settings.brightness < 10) {
                    settings_setBrightness(settings.brightness + 1, true, true);
                    brightness_changed = true;
                    changed = true;
                }
            }

            if (keystate[SW_BTN_DOWN] >= PRESSED){
                // Change brightness
                if (settings.brightness > 0) {
                    settings_setBrightness(settings.brightness - 1, true, true);
                    brightness_changed = true;
                    changed = true;
                }
            }

            if (select_pressed && ((changed_key == SW_BTN_L2 && keystate[SW_BTN_L2] == RELEASED)
                    || (changed_key == SW_BTN_R2 && keystate[SW_BTN_R2] == RELEASED))) {
                settings_load();
                brightness_changed = true;
                changed = true;
            }

            if (changed_key == SW_BTN_SELECT) {
                if (keystate[SW_BTN_SELECT] == PRESSED)
                    select_pressed = true;
                if (keystate[SW_BTN_SELECT] == RELEASED) {
                    if (!select_combo_key) {
                        show_legend = !show_legend;
                        legend_timeout = 0;
                        if (!show_time && !show_total) show_time = true, show_total = false;
                        else if (show_time && !show_total) show_time = true, show_total = true;
                        else show_time = false, show_total = false;
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
                    view_mode = view_mode == VIEW_FULLSCREEN ? view_restore : !view_mode;
                    config_flag_set("gameSwitcher/minimal", view_mode == VIEW_MINIMAL);
                    changed = true;
                }
                button_y_repeat = 0;
            }

            if (keystate[SW_BTN_X] == PRESSED) {
                if (game_list_len != 0) {
                    theme_renderDialog(screen, "Remove from history", "Are you sure you want to\nremove game from history?", true);
                    SDL_BlitSurface(screen, NULL, video, NULL);
                    SDL_Flip(video);
                    sound_change();

                    while (!quit) {
                        if (updateKeystate(keystate, &quit, true, NULL)) {
                            if (keystate[SW_BTN_A] == PRESSED) {
                                removeCurrentItem();
                                readHistory();
                                if (current_game > 0)
                                    current_game--;
                                imageCache_load(&current_game, loadRomscreen, game_list_len);
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

        if (!changed && image_drawn && brightness_changed == false)
            continue;

        if (acc_ticks >= time_step) {
            SDL_BlitSurface(theme_background(), NULL, screen, NULL);

            if (game_list_len == 0) {
                SDL_Surface *empty = resource_getSurface(EMPTY_BG);
                SDL_Rect empty_rect = {320 - empty->w / 2, 240 - empty->h / 2};
                SDL_BlitSurface(empty, NULL, screen, &empty_rect);
                image_drawn = true;
            }
            else {
                SDL_Surface *imageBackgroundGame = imageCache_getItem(&current_game);
                if (imageBackgroundGame != NULL) {
                    if (view_mode == VIEW_NORMAL)
                        SDL_BlitSurface(imageBackgroundGame, &frame, screen, &frame);
                    else
                        SDL_BlitSurface(imageBackgroundGame, NULL, screen, NULL);
                    image_drawn = true;
                }
                else {
                    if (imageCache_isActive())
                        image_drawn = false;
                }
            }

            if (view_mode != VIEW_FULLSCREEN && game_list_len > 0) {
                char *game_name_str = game_list[current_game].shortname;
                SDL_Rect game_name_bg_size = {0, 0, 640, 60};
                SDL_Rect game_name_bg_pos = {0, 360};

                if (view_mode == VIEW_NORMAL) {
                    game_name_bg_size.x = game_name_bg_pos.x = theme()->frame.border_left;
                    game_name_bg_size.w -= theme()->frame.border_right;
                }

                game_name_bg_pos.y = view_mode == VIEW_NORMAL ? (480 - footer_height - 60) : 420;
                SDL_BlitSurface(transparent_bg, &game_name_bg_size, screen, &game_name_bg_pos);

                if (current_game > 0) {
                    SDL_Rect arrow_left_rect = {10, game_name_bg_pos.y + 30 - arrow_left->h / 2};
                    SDL_BlitSurface(arrow_left, NULL, screen, &arrow_left_rect);
                }

                if (current_game < game_list_len - 1) {
                    SDL_Rect arrow_right_rect = {630 - arrow_right->w, game_name_bg_pos.y + 30 - arrow_right->h / 2};
                    SDL_BlitSurface(arrow_right, NULL, screen, &arrow_right_rect);
                }
                
                SDL_Surface *game_name = TTF_RenderUTF8_Blended(resource_getFont(TITLE), game_name_str, color_white);
                game_name_size.w = game_name->w < game_name_max_width ? game_name->w : game_name_max_width;
                game_name_size.h = game_name->h;
                SDL_Rect game_name_rect = {320 - game_name->w / 2, game_name_bg_pos.y + 30 - game_name->h / 2};
                if (game_name_rect.x < game_name_padding)
                    game_name_rect.x = game_name_padding;
                SDL_BlitSurface(game_name, &game_name_size, screen, &game_name_rect);
                SDL_FreeSurface(game_name);
            }

            if (view_mode == VIEW_NORMAL) {
                if (custom_footer) {
                    if (footer_height > 0) {
                        SDL_Rect footer_rect = {0, 480 - custom_footer->h};
                        SDL_BlitSurface(custom_footer, NULL, screen, &footer_rect);
                    }
                }
                else {
                    theme_renderFooter(screen);
                    theme_renderStandardHint(screen, "RESUME", lang_get(LANG_BACK));
                    theme_renderFooterStatus(screen, game_list_len > 0 ? current_game + 1 : 0, game_list_len);
                }
            }

            if (view_mode == VIEW_NORMAL) {
                char title_str[STR_MAX] = "GameSwitcher";
                if (show_time && game_list_len > 0) {
                    strcpy(title_str, game_list[current_game].totalTime);

                    if (show_total) {
                        sprintf(title_str + strlen(title_str), " / %s", sTotalTimePlayed);
                    }
                }

                if (custom_header) {
                    if (header_height > 0) {
                        SDL_BlitSurface(custom_header, NULL, screen, NULL);
                        SDL_Surface *title = TTF_RenderUTF8_Blended(resource_getFont(TITLE), title_str, theme()->title.color);
                        if (title) {
                            SDL_Rect title_rect = {320 - title->w / 2, (header_height - title->h) / 2};
                            SDL_BlitSurface(title, NULL, screen, &title_rect);
                            SDL_FreeSurface(title);
                        }
                        theme_renderHeaderBatteryCustom(screen, battery_percentage, header_height);
                    }
                }
                else {
                    theme_renderHeader(screen, title_str, false);
                    theme_renderHeaderBattery(screen, battery_percentage);
                }
            }

            if (show_legend && view_mode != VIEW_FULLSCREEN) {
                SDL_Surface *legend = resource_getSurface(LEGEND_GAMESWITCHER);
                SDL_Rect legend_rect = {640 - legend->w, view_mode == VIEW_NORMAL ? header_height : 0};
                SDL_BlitSurface(legend, NULL, screen, &legend_rect);
            }

            if (brightness_changed) {
                // Display luminosity slider
                SDL_Surface* brightness = resource_getBrightness(settings.brightness);
                bool vertical = brightness->h > brightness->w;
                SDL_Rect brightness_rect = {0, (view_mode == VIEW_NORMAL ? 240 : 210) - brightness->h / 2};
                if (!vertical) {
                    brightness_rect.x = 320 - brightness->w / 2;
                    brightness_rect.y = view_mode == VIEW_NORMAL ? header_height : 0;
                }
                SDL_BlitSurface(brightness, NULL, screen, &brightness_rect);
            }

            SDL_BlitSurface(screen, NULL, video, NULL);
            SDL_Flip(video);

            changed = false;
			acc_ticks -= time_step;
        }
    }

    screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640,480, 32, 0,0,0,0);

    remove("/mnt/SDCARD/.tmp_update/.runGameSwitcher");
    remove("/mnt/SDCARD/.tmp_update/cmd_to_run.sh");
    if (nExitToMiyoo != 1){
        print_debug("Resuming game");
        FILE *file = fopen("/mnt/SDCARD/.tmp_update/cmd_to_run.sh", "w");
        fputs(game_list[current_game].RACommand, file);
        fclose(file);
    }
    else {
        print_debug("Exiting to menu");
    }

    #ifndef PLATFORM_MIYOOMINI
    msleep(200);
    #endif

    cJSON_free(json_root);

    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);

    if (custom_header != NULL) SDL_FreeSurface(custom_header);
    if (custom_footer != NULL) SDL_FreeSurface(custom_footer);

    resources_free();
    SDL_FreeSurface(transparent_bg);
    imageCache_freeAll();

    SDL_FreeSurface(screen);
    SDL_FreeSurface(video);
    TTF_Quit();
    SDL_Quit();

    return EXIT_SUCCESS;
}
