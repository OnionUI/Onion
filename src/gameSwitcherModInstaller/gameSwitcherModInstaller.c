
#include "sys/ioctl.h"
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "system/keymap_sw.h"
#include "system/lang.h"
#include "system/settings.h"
//include "system/system.h"
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

#define BACKUP_PATH "/mnt/SDCARD/App/GameSwitcherMod/gameSwitcher.bak"
#define ORIGINAL_PATH "/mnt/SDCARD/.tmp_update/bin/gameSwitcher"
#define MOD_PATH "/mnt/SDCARD/App/GameSwitcherMod/gameSwitcherMod"

static bool quit = false;

static void sigHandler(int sig)
{
    switch (sig) {
    case SIGINT:
    case SIGTERM:
        quit = true;
        break;
    default:
        break;
    }
}

static SDL_Surface *video;
static SDL_Surface *screen;
SDL_Surface *arrow_left;
SDL_Surface *arrow_right;

static TTF_Font *font40;
static TTF_Font *font30;

static SDL_Color color_white = {255, 255, 255};
static SDL_Color color_purple = {136, 97, 252};
static SDL_Color color_grey = {117, 123, 156};
static SDL_Color color_lightgrey = {214, 223, 246};

static char messageText[STR_MAX];

void init(void)
{
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    settings_load();
    lang_load();

    SDL_InitDefault(true);

    font40 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 40);
    font30 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 30);

    arrow_left = resource_getSurface(LEFT_ARROW_WB);
    arrow_right = resource_getSurface(RIGHT_ARROW_WB);

    messageText[0] = '\0';
}

void free_resources(void)
{
    resources_free();

    TTF_CloseFont(font40);
    TTF_CloseFont(font30);

    TTF_Quit();

    SDL_FreeSurface(screen);
    SDL_FreeSurface(video);
    SDL_Quit();
}

int renderText(const char *text, TTF_Font *font, SDL_Color color, SDL_Rect *rect)
{
    int text_height = 0;
    SDL_Surface *textSurface = TTF_RenderUTF8_Blended(font, text, color);

    if (textSurface != NULL) {
        text_height = textSurface->h;
        SDL_BlitSurface(textSurface, NULL, screen, rect);
        SDL_FreeSurface(textSurface);
    }
    return text_height;
}

int renderTextBox(const char *text, TTF_Font *font, SDL_Color color, SDL_Rect *rect)
{
    int text_height = 0;
    SDL_Surface *textSurface = theme_textboxSurface(text, font, color, ALIGN_LEFT);

    if (textSurface != NULL) {
        text_height = textSurface->h;
        SDL_BlitSurface(textSurface, NULL, screen, rect);
        SDL_FreeSurface(textSurface);
    }
    return text_height;
}

void renderPage(int current_page)
{
    char titleLine[STR_MAX];
    char descLine[STR_MAX];

    switch (current_page) {
    case 0:
        sprintf(titleLine, "Install");
        sprintf(descLine, "Make a backup of the original\nGame Switcher and replace it\nwith a modified version.");
        break;

    case 1:
        sprintf(titleLine, "Uninstall");
        sprintf(descLine, "Restore the original\nGame Switcher from a backup.");
        break;

    case 2:
        sprintf(titleLine, "Reinstall");
        sprintf(descLine, "Replace the current Game Switcher\nwith a modified version.\nLeave the backup unchanged\nand not create a new one.");
        break;

    default:
        break;
    }

    renderText(titleLine, font40, color_purple, &(SDL_Rect){50, 80});
    renderTextBox(descLine, font30, color_white, &(SDL_Rect){50, 80 + 40});
    if (exists(BACKUP_PATH)) {
        renderText("Backup found.", font30, color_lightgrey, &(SDL_Rect){50, 80 + 220});
    }
    if (strlen(messageText) > 0) {
        renderText(messageText, font30, color_grey, &(SDL_Rect){50, 80 + 280});
        messageText[0] = '\0';
    }
}

int main(int argc, char *argv[])
{
    log_setName("gameSwitcherMod");

    init();

    SDL_BlitSurface(theme_background(), NULL, screen, NULL);

    int num_pages = 3;
    int current_page = 0;

    renderPage(current_page);

    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);

    bool changed = true;
    KeyState keystate[320] = {(KeyState)0};

    while (!quit) {
        if (updateKeystate(keystate, &quit, true, NULL)) {
            if (keystate[SW_BTN_A] == PRESSED) {
                switch (current_page) {
                case 0: // Install
                {
                    bool canInstall = true;

                    if (exists(BACKUP_PATH)) {
                        canInstall = false;

                        theme_renderDialog(
                            screen, "Backup found",
                            "Warning!\nThe current backup will be replaced.\nContinue?",
                            true);

                        SDL_BlitSurface(screen, NULL, video, NULL);
                        SDL_Flip(video);
                        sound_change();

                        while (!quit) {
                            if (updateKeystate(keystate, &quit, true, NULL)) {
                                if (keystate[SW_BTN_A] == PRESSED) {
                                    canInstall = true;
                                    break;
                                }
                                if (keystate[SW_BTN_B] == PRESSED) {
                                    keystate[SW_BTN_B] = RELEASED;
                                    break;
                                }
                            }
                        }
                    }

                    if (canInstall) {
                        if (exists(MOD_PATH)) {
                            if (exists(ORIGINAL_PATH))
                                file_copy(ORIGINAL_PATH, BACKUP_PATH);
                            file_copy(MOD_PATH, ORIGINAL_PATH);
                            strcpy(messageText, "Done.");
                        }
                        else {
                            strcpy(messageText, "Something went wrong.");
                        }
                    }
                } break;

                case 1: // Uninstall
                {
                    if (exists(BACKUP_PATH)) {
                        file_copy(BACKUP_PATH, ORIGINAL_PATH);
                        remove(BACKUP_PATH);
                        strcpy(messageText, "Done.");
                    }
                    else {
                        strcpy(messageText, "Error: Backup is missing.");
                    }
                } break;

                case 2: // Reinstall
                {
                    bool canInstall = true;

                    if (!exists(BACKUP_PATH)) {
                        canInstall = false;

                        theme_renderDialog(
                            screen, "Backup not found",
                            "Warning!\nYou do not have a backup,\nuninstalling will not be possible.\nContinue?",
                            true);

                        SDL_BlitSurface(screen, NULL, video, NULL);
                        SDL_Flip(video);
                        sound_change();

                        while (!quit) {
                            if (updateKeystate(keystate, &quit, true, NULL)) {
                                if (keystate[SW_BTN_A] == PRESSED) {
                                    canInstall = true;
                                    break;
                                }
                                if (keystate[SW_BTN_B] == PRESSED) {
                                    keystate[SW_BTN_B] = RELEASED;
                                    break;
                                }
                            }
                        }
                    }

                    if (canInstall) {
                        if (exists(MOD_PATH)) {
                            file_copy(MOD_PATH, ORIGINAL_PATH);
                            strcpy(messageText, "Done.");
                        }
                        else {
                            strcpy(messageText, "Something went wrong.");
                        }
                    }
                } break;

                default:
                    break;
                }

                changed = true;
            }
            if (keystate[SW_BTN_B] == PRESSED) {
                quit = true;
            }
            if (keystate[SW_BTN_RIGHT] >= PRESSED) {
                if (current_page < num_pages - 1) {
                    current_page++;
                    changed = true;
                }
            }
            if (keystate[SW_BTN_LEFT] >= PRESSED) {
                if (current_page > 0) {
                    current_page--;
                    changed = true;
                }
            }
        }

        if (changed) {
            SDL_BlitSurface(theme_background(), NULL, screen, NULL);

            theme_renderFooter(screen);

            theme_renderStandardHint(
                screen, lang_get(LANG_OK, LANG_FALLBACK_OK),
                lang_get(LANG_EXIT, LANG_FALLBACK_EXIT));

            theme_renderFooterStatus(
                screen, num_pages > 0 ? current_page + 1 : 0,
                num_pages);

            renderPage(current_page);

            if (current_page > 0) {
                SDL_Rect arrow_left_rect = {theme()->frame.border_left + 10, 240 - arrow_left->h / 2};
                SDL_BlitSurface(arrow_left, NULL, screen, &arrow_left_rect);
            }

            if (current_page < num_pages - 1) {
                SDL_Rect arrow_right_rect = {630 - theme()->frame.border_right - arrow_right->w, 240 - arrow_right->h / 2};
                SDL_BlitSurface(arrow_right, NULL, screen, &arrow_right_rect);
            }

            theme_renderHeader(screen, "Game Switcher Mod Installer", false);

            SDL_BlitSurface(screen, NULL, video, NULL);
            SDL_Flip(video);

            sound_change();

            changed = false;
        }
    }

    free_resources();

    return EXIT_SUCCESS;
}
