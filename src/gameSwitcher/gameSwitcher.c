#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <fcntl.h>
#include <libgen.h>
#include <linux/fb.h>
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
#include "system/lang.h"
#include "system/settings.h"
#include "system/state.h"
#include "theme/background.h"
#include "theme/sound.h"
#include "theme/theme.h"
#include "utils/config.h"
#include "utils/msleep.h"
#include "utils/surfaceSetAlpha.h"

#include "../playActivity/playActivityDB.h"

#include "gs_history.h"
#include "gs_overlay.h"

#define VIEW_NORMAL 0
#define VIEW_MINIMAL 1
#define VIEW_FULLSCREEN -1

static bool quit = false;
static bool exit_to_menu = false;

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

static int current_game = 0;

static SDL_Surface *surfaceGameName = NULL;
static int gameNameScrollX = 0;
static int gameNameScrollSpeed = 10;
static int gameNameScrollStart = 20;
static int gameNameScrollEnd = 20;

void removeCurrentItem()
{
    Game_s *game = &game_list[current_game];

    printf_debug("removing: %s\n", game->name);
    printf_debug("linenumber: %i\n", game->recentItem.lineNo);

    if (game->romScreen != NULL) {
        SDL_FreeSurface(game->romScreen);
        game->romScreen = NULL;
    }

    file_delete_line(getMiyooRecentFilePath(), game->recentItem.lineNo);

    if (strlen(game->recentItem.imgpath) > 0 && is_file(game->recentItem.imgpath)) {
        if (strncmp(game->recentItem.imgpath, ROM_SCREENS_DIR, strlen(ROM_SCREENS_DIR)) == 0) {
            remove(game->recentItem.imgpath);
        }
    }

    // Copy next element value to current element
    for (int i = current_game; i < game_list_len - 1; i++) {
        game_list[i] = game_list[i + 1];
        game_list[i].recentItem.lineNo -= 1;
        game_list[i].index -= 1;
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

void renderCentered(SDL_Surface *image, int view_mode, SDL_Rect *overrideSrcRect, SDL_Rect *overrideDestRect)
{
    int offSetX = (int)(g_display.width - image->w) / 2;
    int offSetY = (int)(g_display.height - image->h) / 2;

    SDL_Rect image_size = {0, 0, g_display.width, g_display.height};
    SDL_Rect image_pos = {offSetX, offSetY};

    if (view_mode == VIEW_NORMAL) {
        image_size.x = theme()->frame.border_left;
        image_size.w -= theme()->frame.border_left + theme()->frame.border_right;
        image_pos.x += theme()->frame.border_left;
    }

    if (overrideSrcRect != NULL) {
        image_size = *overrideSrcRect;
        image_size.x -= offSetX;
        image_size.y -= offSetY;
    }

    if (overrideDestRect != NULL) {
        image_pos = *overrideDestRect;
    }

    SDL_BlitSurface(image, &image_size, screen, &image_pos);
}

SDL_Surface *loadOptionalImage(const char *resourceName)
{
    char image_path[STR_MAX];
    if (theme_getImagePath(theme()->path, resourceName, image_path)) {
        return theme_loadImage(theme()->path, resourceName);
    }
    return NULL;
}

int getHeightOrDefault(SDL_Surface *surface, int default_height)
{
    int height = surface ? surface->h : default_height;
    return height > 1 ? height : 0;
}

int main(int argc, char *argv[])
{
    const bool is_overlay = argc > 1 && strcmp(argv[1], "--overlay") == 0;

    log_setName("gameSwitcher");
    print_debug("\n\nDebug logging enabled");

    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    init(INIT_ALL);

    g_scale = (double)g_display.width / 640.0;
    scaleSurfaceFunc = zoomSurface;

    readFirstEntry();

    overlay_init(is_overlay);

    loadRomScreens();

    settings_load();
    lang_load();

    SDL_Color color_white = {255, 255, 255};
    SDL_Surface *transparent_bg = SDL_CreateRGBSurface(0, g_display.width, g_display.height, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    SDL_FillRect(transparent_bg, NULL, 0xBE000000);

    SDL_Surface *arrow_left = resource_getSurface(LEFT_ARROW_WB);
    SDL_Surface *arrow_right = resource_getSurface(RIGHT_ARROW_WB);
    int game_name_padding = arrow_left->w + 20;
    int game_name_max_width = g_display.width - 2 * game_name_padding;
    SDL_Rect game_name_size = {0, 0};

    int battery_percentage = battery_getPercentage();

    bool changed = true;
    bool current_game_changed = true;
    bool brightness_changed = false;

    KeyState keystate[320] = {(KeyState)0};
    bool btn_a_pressed = false;
    bool menu_pressed = false;
    bool combo_key = false;
    bool select_pressed = false;
    bool select_combo_key = false;

    mkdirs("/mnt/SDCARD/.tmp_update/config/gameSwitcher");

    bool view_min = config_flag_get("gameSwitcher/minimal");
    bool show_time = config_flag_get("gameSwitcher/showTime");
    bool show_total = !config_flag_get("gameSwitcher/hideTotal");
    bool show_legend = !config_flag_get("gameSwitcher/hideLegend");
    int view_mode = view_min ? VIEW_MINIMAL : VIEW_NORMAL;
    int view_restore = view_mode;

    SDLKey changed_key = SDLK_UNKNOWN;
    int button_y_repeat = 0;

    uint32_t acc_ticks = 0, last_ticks = SDL_GetTicks(), time_step = 1000 / 30;

    uint32_t legend_start = last_ticks;
    uint32_t legend_timeout = 5000;

    uint32_t brightness_start = last_ticks;
    uint32_t brightness_timeout = 2000;

    SDL_Surface *custom_header = loadOptionalImage("extra/gs-top-bar");
    SDL_Surface *custom_footer = loadOptionalImage("extra/gs-bottom-bar");

    int header_height = getHeightOrDefault(custom_header, 60.0 * g_scale);
    int footer_height = getHeightOrDefault(custom_footer, 60.0 * g_scale);

    SDL_Surface *current_bg = NULL;
    bool first_render = true;

    print_debug("gameSwitcher started\n");

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

        if (_updateKeystate(keystate, &quit, true, &changed_key)) {
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

            if (keystate[SW_BTN_A] == PRESSED) {
                btn_a_pressed = true;
            }
            else if (keystate[SW_BTN_A] == RELEASED && btn_a_pressed) {
                btn_a_pressed = false;
                break;
            }

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
                    render();
                    sound_change();

                    while (!quit) {
                        if (_updateKeystate(keystate, &quit, true, NULL)) {
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

            Game_s *game = &game_list[current_game];
            processItem(game);

            if (changed) {
                SDL_FillRect(screen, NULL, 0);

                if (game_list_len == 0) {
                    current_bg = NULL;
                    SDL_Surface *empty = resource_getSurface(EMPTY_BG);
                    SDL_Rect empty_rect = {(g_display.width - empty->w) / 2, (g_display.height - empty->h) / 2};
                    SDL_BlitSurface(empty, NULL, screen, &empty_rect);
                }
                else {
                    current_bg = loadRomScreen(current_game);

                    if (current_bg != NULL) {
                        renderCentered(current_bg, view_mode, NULL, NULL);
                    }
                }
            }

            if (view_mode != VIEW_FULLSCREEN && game_list_len > 0) {
                SDL_Rect game_name_bg_size = theme_scaleRect((SDL_Rect){0, 0, 640, 60});
                SDL_Rect game_name_bg_pos = {0, 0};

                if (view_mode == VIEW_NORMAL) {
                    game_name_bg_size.x = game_name_bg_pos.x = theme()->frame.border_left;
                    game_name_bg_size.w -= theme()->frame.border_left + theme()->frame.border_right;
                }

                int name_pos = g_display.height - game_name_bg_size.h;
                if (view_mode == VIEW_NORMAL) {
                    name_pos -= footer_height;
                }
                game_name_bg_size.y = game_name_bg_pos.y = name_pos;

                game_name_bg_pos.w = game_name_bg_size.w;
                game_name_bg_pos.h = game_name_bg_size.h;

                SDL_FillRect(screen, &game_name_bg_pos, 0);

                if (current_bg != NULL) {
                    renderCentered(current_bg, view_mode, &game_name_bg_size, &game_name_bg_pos);
                }

                SDL_BlitSurface(transparent_bg, &game_name_bg_size, screen, &game_name_bg_pos);

                if (current_game > 0) {
                    SDL_Rect arrow_left_rect = {(double)(theme()->frame.border_left + 10) * g_scale, 30.0 * g_scale - arrow_left->h / 2};
                    arrow_left_rect.y += game_name_bg_pos.y;
                    SDL_BlitSurface(arrow_left, NULL, screen, &arrow_left_rect);
                }

                if (current_game < game_list_len - 1) {
                    SDL_Rect arrow_right_rect = {
                        (double)(630 - theme()->frame.border_right) * g_scale - arrow_right->w,
                        30.0 * g_scale - arrow_right->h / 2};
                    arrow_right_rect.y += game_name_bg_pos.y;
                    SDL_BlitSurface(arrow_right, NULL, screen, &arrow_right_rect);
                }

                char game_name_str[STR_MAX * 2 + 4];
                strcpy(game_name_str, game->shortname);

                if (current_game_changed) {
                    if (surfaceGameName != NULL)
                        SDL_FreeSurface(surfaceGameName);
                    surfaceGameName = TTF_RenderUTF8_Blended(resource_getFont(TITLE), game_name_str, color_white);
                    game_name_size.w = surfaceGameName->w < game_name_max_width ? surfaceGameName->w : game_name_max_width;
                    game_name_size.h = surfaceGameName->h;
                    gameNameScrollX = -gameNameScrollStart * gameNameScrollSpeed;
                }

                SDL_Rect game_name_rect = {(g_display.width - surfaceGameName->w) / 2,
                                           game_name_bg_pos.y + 30.0 * g_scale - surfaceGameName->h / 2};
                if (game_name_rect.x < game_name_padding)
                    game_name_rect.x = game_name_padding;

                game_name_size.x =
                    gameNameScrollX < (surfaceGameName->w - game_name_size.w)
                        ? (gameNameScrollX > 0 ? gameNameScrollX : 0)
                        : surfaceGameName->w - game_name_size.w;

                SDL_BlitSurface(surfaceGameName, &game_name_size, screen, &game_name_rect);

                if (surfaceGameName->w > game_name_max_width) {
                    gameNameScrollX += gameNameScrollSpeed;

                    if (gameNameScrollX > (surfaceGameName->w - game_name_size.w + gameNameScrollEnd * gameNameScrollSpeed))
                        gameNameScrollX = -gameNameScrollStart * gameNameScrollSpeed;
                }
            }

            if (!changed) {
                render();
                continue;
            }

            if (view_mode == VIEW_NORMAL) {
                if (custom_footer) {
                    if (footer_height > 0) {
                        SDL_Rect footer_rect = {0, g_display.height - custom_footer->h};
                        SDL_BlitSurface(custom_footer, NULL, screen, &footer_rect);
                    }
                }
                else {
                    theme_renderFooter(screen);
                    theme_renderStandardHint(screen,
                                             lang_get(LANG_RESUME_UC, LANG_FALLBACK_RESUME_UC),
                                             lang_get(LANG_BACK, LANG_FALLBACK_BACK));
                    if (!first_render) {
                        theme_renderFooterStatus(screen,
                                                 game_list_len > 0 ? current_game + 1 : 0,
                                                 game_list_len);
                    }
                }
            }

            if (view_mode == VIEW_NORMAL) {
                char title_str[STR_MAX] = "GameSwitcher";
                if (show_time && game_list_len > 0) {
                    if (strlen(game->totalTime) == 0) {
                        str_serializeTime(game->totalTime, play_activity_get_play_time(game->recentItem.rompath));
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
                            SDL_Rect title_rect = {(g_display.width - title->w) / 2,
                                                   (header_height - title->h) / 2};
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
                if (legend) {
                    printf_debug("Legend size: %i x %i\n", legend->w, legend->h);
                    SDL_Rect legend_rect = {g_display.width - legend->w,
                                            view_mode == VIEW_NORMAL ? header_height : 0};
                    printf_debug("Displaying legend at %i, %i\n", legend_rect.x, legend_rect.y);
                    SDL_BlitSurface(legend, NULL, screen, &legend_rect);
                }
            }

            if (brightness_changed) {
                // Display luminosity slider
                SDL_Surface *brightness = resource_getBrightness(settings.brightness);
                bool vertical = brightness->h > brightness->w;
                SDL_Rect brightness_rect = {0, (double)(view_mode == VIEW_NORMAL ? 240 : 210) * g_scale - brightness->h / 2};
                if (!vertical) {
                    brightness_rect.x = (g_display.width - brightness->w) / 2;
                    brightness_rect.y = view_mode == VIEW_NORMAL ? header_height : 0;
                }
                SDL_BlitSurface(brightness, NULL, screen, &brightness_rect);
            }

            render();

            if (first_render) {
                first_render = false;
                readHistory();
                loadRomScreens();
            }
            else {
                changed = false;
                current_game_changed = false;
            }
        }
    }

    if (exit_to_menu) {
        print_debug("Exiting to menu");
        remove("/mnt/SDCARD/.tmp_update/.runGameSwitcher");
        remove("/mnt/SDCARD/.tmp_update/cmd_to_run.sh");
        overlay_exit();
    }
    else if (is_overlay && current_game == 0) {
        if (current_bg != NULL) {
            SDL_FillRect(screen, NULL, 0);
            renderCentered(current_bg, VIEW_FULLSCREEN, NULL, NULL);
        }
        overlay_resume();
    }
    else {
        printf_debug("Resuming game - current_game : %i - index: %i", current_game, game_list[current_game].index);
        resumeGame(game_list[current_game].index);
        overlay_exit();
    }

#ifndef PLATFORM_MIYOOMINI
    msleep(200);
#endif

    if (custom_header != NULL)
        SDL_FreeSurface(custom_header);
    if (custom_footer != NULL)
        SDL_FreeSurface(custom_footer);
    if (surfaceGameName != NULL)
        SDL_FreeSurface(surfaceGameName);

    resources_free();
    SDL_FreeSurface(transparent_bg);

    freeRomScreens();
    ra_freeHistory();

    deinit();

    return EXIT_SUCCESS;
}
