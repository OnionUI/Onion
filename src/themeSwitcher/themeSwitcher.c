#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "sys/ioctl.h"
#include <dirent.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#include "utils/file.h"
#include "utils/json.h"
#include "utils/msleep.h"
#include "utils/log.h"
#include "system/settings.h"
#include "system/keymap_sw.h"
#include "theme/config.h"

#ifndef DT_REG
#define DT_REG 8
#endif
#ifndef DT_DIR
#define DT_DIR 4
#endif

// Max number of records in the DB
#define NUMBER_OF_THEMES 100
#define MAX_THEME_NAME_SIZE 256

#define SYSTEM_LANG_DIR "/mnt/SDCARD/miyoo/app/lang"
#define SYSTEM_LANG_BACKUP_DIR "/mnt/SDCARD/miyoo/app/lang_backup"
#define SYSTEM_SKIN_DIR "/mnt/SDCARD/miyoo/app/skin"

static bool quit = false;

void removeIconTitles(void)
{
    DIR *dp;
    struct dirent *ep;
    FILE *fp;

	if ((dp = opendir(SYSTEM_LANG_DIR)) == NULL)
        return;

    while ((ep = readdir(dp))) {
        if (ep->d_type != DT_REG)
            continue; // skip non-regular files and folders
        if (strcmp("lang", file_getExtension(ep->d_name)) != 0)
            continue; // skip files not having the `.lang` extension

        const char file_path[256];
        sprintf(file_path, SYSTEM_LANG_DIR "/%s", ep->d_name);

        const char *json_data = file_read(file_path);
        cJSON *root = cJSON_Parse(json_data);

        if (!root)
            continue;

        printf_debug("Lang: %s (%s)\n", cJSON_GetStringValue(cJSON_GetObjectItem(root, "lang")), ep->d_name);

        // Remove icon titles
        json_setString(root, "0", " "); // Expert
        json_setString(root, "1", " "); // Favorites
        json_setString(root, "2", " "); // Games
        json_setString(root, "15", " "); // Settings
        json_setString(root, "18", " "); // Recents
        json_setString(root, "107", " "); // Apps

        // Remove hints
        json_setString(root, "88", " "); // SELECT
        json_setString(root, "89", " "); // BACK
        json_setString(root, "111", " "); // EXIT
        json_setString(root, "112", " "); // SAVE AND EXIT

        json_save(root, file_path);
        cJSON_free(root);
    }
    closedir(dp);
}

void installFaultyImage(const char *theme_path, const char *image_name)
{
    char override_image_path[256],
         theme_image_path[256],
         system_image_path[256],
         system_image_backup[256];

    sprintf(override_image_path, "%sskin/%s.png", THEME_OVERRIDES, image_name);
    sprintf(theme_image_path, "%sskin/%s.png", theme_path, image_name);
    sprintf(system_image_path, SYSTEM_SKIN_DIR "/%s.png", image_name);
    sprintf(system_image_backup, SYSTEM_SKIN_DIR "/%s_back.png", image_name);

    // backup system skin
    if (!exists(system_image_backup))
        file_copy(system_image_path, system_image_backup);

    if (exists(override_image_path)) {
        // apply override image
        file_copy(override_image_path, system_image_path);
    }
    else if (exists(theme_image_path)) {
        // apply theme image
        file_copy(theme_image_path, system_image_path);
    }
    else {
        // restore system skin
        file_copy(system_image_backup, system_image_path);
        remove(system_image_backup);
    }
}

void installTheme(const char *theme_name, bool hideIconTitle)
{
    system("./bin/mainUiBatPerc --restore");

    // change theme setting
    sprintf(settings.theme, "/mnt/SDCARD/Themes/%s/", theme_name);
    settings_save();

    if (hideIconTitle) {
        if (!exists(SYSTEM_LANG_BACKUP_DIR)) {
            // backup lang files
            system("cp -R " SYSTEM_LANG_DIR " " SYSTEM_LANG_BACKUP_DIR "");
            // remove icon titles in current lang files
            removeIconTitles();
        }
    }
    else if (exists(SYSTEM_LANG_BACKUP_DIR)) {
        // restore original lang files
        system("mv -f " SYSTEM_LANG_BACKUP_DIR "/* " SYSTEM_LANG_DIR "");
        remove(SYSTEM_LANG_BACKUP_DIR);
    }

    installFaultyImage(settings.theme, "bg-io-testing");
    installFaultyImage(settings.theme, "ic-MENU+A");
}

int main(void)
{
	DIR *dp;
	struct dirent *ep;

	int themes_count = 0;
	char themes[NUMBER_OF_THEMES][MAX_THEME_NAME_SIZE];
    char theme_path[512];
    int current_page = 0;
    int hideIconTitle = 0;

	settings_load();
	char *installed_theme = basename(settings.theme);
    int installed_page = 0;

	if ((dp = opendir("/mnt/SDCARD/Themes")) != NULL) {
        while ((ep = readdir(dp))) {
            if (ep->d_type != DT_DIR) continue;

            sprintf(theme_path, "/mnt/SDCARD/Themes/%s/config.json", ep->d_name);
            
            if (exists(theme_path)) {
                strcpy(themes[themes_count], ep->d_name);
				if (strcmp(ep->d_name, installed_theme) == 0)
					current_page = installed_page = themes_count;
                themes_count++;
            }
        }
        closedir(dp);
	}
	else {
		perror("Couldn't open the Themes directory");
	}

    SDL_Init(SDL_INIT_VIDEO);
    SDL_ShowCursor(SDL_DISABLE);
	SDL_EnableKeyRepeat(300, 50);
    TTF_Init();

    SDL_Surface* video = SDL_SetVideoMode(640,480, 32, SDL_HWSURFACE);
    SDL_Surface* screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640,480, 32, 0,0,0,0);
    TTF_Font* font40 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 40);
    TTF_Font* font21 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 21);
    TTF_Font* font30 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 30);

    SDL_Color color_white = {255, 255, 255};

    SDL_Surface* background_page0 = IMG_Load("res/background.png");
    SDL_Surface* background_page1 = IMG_Load("res/themeDetail.png");

    SDL_Surface* imagePages;
    SDL_Surface* imageThemeNom;

    SDL_Surface* surfaceArrowLeft = IMG_Load("res/arrowLeft.png");
    SDL_Surface* surfaceArrowRight = IMG_Load("res/arrowRight.png");

    SDL_Rect preview_src_rect = {0, 0, 480, 360};
    SDL_Rect rectArrowLeft = {24, 210, 28, 32};
    SDL_Rect rectArrowRight = {588, 210, 28, 32};
    SDL_Rect rectPages = {559, 450};
    SDL_Rect rectThemeDescription = {10, 175, 600, 44};
    SDL_Rect rectThemePreview = {80, 46, 480, 360};
    SDL_Rect rectImageThemeNom = {77, 7, 557, 54};
    int levelPage = 0;
    FILE *fp;
    long lSize;

    SDL_FillRect(screen, NULL, 0);

    SDL_Surface *loading = TTF_RenderUTF8_Blended(font30, "Loading previews...", color_white);
    SDL_Rect loadingRect = {320 - loading->w / 2, 240 - loading->h / 2};
    SDL_BlitSurface(loading, NULL, screen, &loadingRect);
    SDL_FreeSurface(loading);

    SDL_Surface *previews[themes_count];
    int i;
    for (i = 0; i < themes_count; i++) {
        sprintf(theme_path, "/mnt/SDCARD/Themes/%s/preview.png", themes[i]);
        previews[i] = IMG_Load(exists(theme_path) ? theme_path : "res/noThemePreview.png");
    }

    char cPages[10];

    SDL_Event event;
    Uint8 keystate[320] = {0};
    bool changed = true;

    while (!quit) {
		while (SDL_PollEvent(&event)) {
            SDLKey key = event.key.keysym.sym;
			switch (event.type) {
                case SDL_QUIT: quit = true; break;
                case SDL_KEYDOWN:
                    keystate[key] = keystate[key] != 0 ? 2 : 1;
                    changed = true;
                    break;
                case SDL_KEYUP:
                    keystate[key] = 0;
                    changed = true;
                    break;
                default: break;
			}
		}

        if (!changed)
            continue;

        if (keystate[SW_BTN_B]) {
            if (levelPage == 0) quit = true; // exit program
            else levelPage = 0;
        }

        if (keystate[SW_BTN_A]) {
            if (levelPage == 1) {
                // Install theme
                installTheme(themes[current_page], hideIconTitle);
                printf_debug("Theme installed: %s\n", themes[current_page]);
                quit = true;
            }
            else {
                // Go to theme details/confirmation page
                levelPage = 1;
            }
        }

        if (keystate[SW_BTN_RIGHT]) {
            if (current_page < (themes_count-1)){
                current_page ++;
            }
        }
        if (keystate[SW_BTN_LEFT]) {
            if (current_page > 0){
                current_page --;
            }
        }

        if (quit)
            break;

        if (levelPage == 0) {
            SDL_BlitSurface(previews[current_page], &preview_src_rect, screen, &rectThemePreview);
            SDL_BlitSurface(background_page0, NULL, screen, NULL);

            if (current_page != 0) {
                SDL_BlitSurface(surfaceArrowLeft, NULL, screen, &rectArrowLeft);
            }
            if (current_page != themes_count - 1) {
                SDL_BlitSurface(surfaceArrowRight, NULL, screen, &rectArrowRight);
            }

            sprintf(cPages,"%d/%d",(current_page+1),themes_count);
            imagePages = TTF_RenderUTF8_Blended(font30, cPages, color_white);
            rectPages.x = 620 - imagePages->w;
            rectPages.y = 450 - imagePages->h / 2;
            SDL_BlitSurface(imagePages, NULL, screen, &rectPages);
	        SDL_FreeSurface(imagePages);

            sprintf(theme_path, "/mnt/SDCARD/Themes/%s/config.json", themes[current_page]);

            if (exists(theme_path)) {
                const char *json_data = file_read(theme_path);

                cJSON* request_json = NULL;
                cJSON* themeName;

                if (json_data != NULL) {
                    request_json = cJSON_Parse(json_data);
                    themeName = cJSON_GetObjectItem(request_json, "name");
                    char title[256];
                    if (current_page == installed_page)
                        sprintf(title, "%s - Installed", cJSON_GetStringValue(themeName));
                    else
                        sprintf(title, "%s", cJSON_GetStringValue(themeName));
                    imageThemeNom = TTF_RenderUTF8_Blended(font21, title, color_white);
                    SDL_BlitSurface(imageThemeNom, NULL, screen, &rectImageThemeNom);
                }
            }
        }
        else {
            SDL_BlitSurface(background_page1, NULL, screen, NULL);

            sprintf(theme_path, "/mnt/SDCARD/Themes/%s/config.json", themes[current_page]);

            if (exists(theme_path)) {
                const char *json_data = file_read(theme_path);

                cJSON* request_json = NULL;
                cJSON* themeName;
                cJSON* themeAuthor;
                cJSON* themeIconTitle;
                cJSON* themeLang;

                if (json_data != NULL){
                    request_json = cJSON_Parse(json_data);

                    themeName = cJSON_GetObjectItem(request_json, "name");
                    themeAuthor = cJSON_GetObjectItem(request_json, "author");
                    themeIconTitle = cJSON_GetObjectItem(request_json, "hideIconTitle");

                    if (cJSON_IsTrue(themeIconTitle))
						hideIconTitle = 1;

                    char title[256];
                    if (themeAuthor)
                        sprintf(title, "%s by %s", cJSON_GetStringValue(themeName), cJSON_GetStringValue(themeAuthor));
                    else
                        sprintf(title, "%s", cJSON_GetStringValue(themeName));

                    imagePages = TTF_RenderUTF8_Blended(font40, title, color_white);
                    SDL_BlitSurface(imagePages, NULL, screen, &rectThemeDescription);
	                SDL_FreeSurface(imagePages);
                }
            }
        }

        SDL_BlitSurface(screen, NULL, video, NULL);
        SDL_Flip(video);

        changed = false;
    }

    msleep(100);

    for (i = 0; i < themes_count; i++)
        SDL_FreeSurface(previews[i]);
	SDL_FreeSurface(surfaceArrowLeft);
	SDL_FreeSurface(surfaceArrowRight);
	SDL_FreeSurface(background_page0);
	SDL_FreeSurface(background_page1);
    SDL_Quit();

    return EXIT_SUCCESS;
}
