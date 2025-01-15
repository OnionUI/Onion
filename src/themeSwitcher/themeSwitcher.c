#include "sys/ioctl.h"
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "system/keymap_sw.h"
#include "system/lang.h"
#include "system/settings.h"
#include "utils/msleep.h"

#include "installTheme.h"

static bool quit = false;

void showCenteredMessage(SDL_Surface *video, SDL_Surface *screen,
                         const char *message_str, TTF_Font *font,
                         SDL_Color color)
{
    SDL_Surface *message = TTF_RenderUTF8_Blended(font, message_str, color);
    SDL_Rect loadingRect = {320 - message->w / 2, 240 - message->h / 2};
    SDL_FillRect(screen, NULL, 0);
    SDL_BlitSurface(message, NULL, screen, &loadingRect);
    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);
    SDL_FreeSurface(message);
}

SDL_Surface *createBottomBar(TTF_Font *font)
{
    SDL_Surface *surfaceButtonA = IMG_Load("res/button_a.png");
    SDL_Surface *surfaceButtonB = IMG_Load("res/button_b.png");
    SDL_Surface *surfaceButtonX = IMG_Load("res/button_x.png");

    SDL_Surface *surface = SDL_CreateRGBSurface(0, 640, 70, 32, 0, 0, 0, 0);

    SDL_FillRect(surface, NULL, 0);

    SDL_Rect pos = {20, 35 - surfaceButtonA->h / 2};
    SDL_BlitSurface(surfaceButtonA, NULL, surface, &pos);
    pos.x += surfaceButtonA->w + 10;

    SDL_Surface *text =
        TTF_RenderUTF8_Blended(font, "INSTALL", (SDL_Color){255, 255, 255});
    pos.y = 35 - text->h / 2 - 3;
    SDL_BlitSurface(text, NULL, surface, &pos);
    pos.x += text->w + 20;
    SDL_FreeSurface(text);

    pos.y = 35 - surfaceButtonB->h / 2;
    SDL_BlitSurface(surfaceButtonB, NULL, surface, &pos);
    pos.x += surfaceButtonB->w + 10;

    text = TTF_RenderUTF8_Blended(font, "CANCEL", (SDL_Color){255, 255, 255});
    pos.y = 35 - text->h / 2 - 3;
    SDL_BlitSurface(text, NULL, surface, &pos);
    pos.x += text->w + 20;
    SDL_FreeSurface(text);

    pos.y = 35 - surfaceButtonX->h / 2;
    SDL_BlitSurface(surfaceButtonX, NULL, surface, &pos);
    pos.x += surfaceButtonX->w + 10;

    text = TTF_RenderUTF8_Blended(font, "TOGGLE ICONS",
                                  (SDL_Color){255, 255, 255});
    pos.y = 35 - text->h / 2 - 3;
    SDL_BlitSurface(text, NULL, surface, &pos);
    pos.x += text->w + 20;
    SDL_FreeSurface(text);

    SDL_FreeSurface(surfaceButtonA);
    SDL_FreeSurface(surfaceButtonB);
    SDL_FreeSurface(surfaceButtonX);

    return surface;
}

SDL_Surface *getPreviewIcon(const char *path, SDL_Surface *file_zip,
                            SDL_Surface *file_7z, SDL_Surface *file_rar)
{
    char source_path[STR_MAX];
    snprintf(source_path, STR_MAX - 1, "%s/source", path);

    if (!is_file(source_path))
        return NULL;

    FILE *fp;
    file_get(fp, source_path, "%[^\n]", source_path);

    const char *ext = file_getExtension(source_path);

    if (strcmp("zip", ext) == 0)
        return file_zip;
    if (strcmp("7z", ext) == 0)
        return file_7z;
    if (strcmp("rar", ext) == 0)
        return file_rar;

    return NULL;
}

int main(int argc, char *argv[])
{
    settings_load();
    const char *installed_theme = basename(settings.theme);

    bool reapply = false;
    bool reapply_icons = false;
    bool update_previews = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--reapply") == 0) {
            reapply = true;
        }
        else if (strcmp(argv[i], "--reapply_icons") == 0) {
            reapply = true;
            reapply_icons = true;
        }
        else if (strcmp(argv[i], "--update") == 0) {
            reapply = true;
            update_previews = true;
        }
    }

    if (reapply) {
        reinstallTheme(installed_theme, reapply_icons, update_previews);
        return 0;
    }

    SDL_Init(SDL_INIT_VIDEO);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_EnableKeyRepeat(300, 50);
    TTF_Init();

    SDL_Surface *video = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE);
    SDL_Surface *screen =
        SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);

    TTF_Font *font40 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 40);
    TTF_Font *font21 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 21);
    TTF_Font *font30 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 30);

    SDL_Color color_white = {255, 255, 255};

    SDL_Surface *background_page0 = IMG_Load("res/background.png");

    SDL_Surface *imagePages;
    SDL_Surface *imageThemeNom;

    SDL_Surface *surfaceArrowLeft = IMG_Load("res/arrowLeft.png");
    SDL_Surface *surfaceArrowRight = IMG_Load("res/arrowRight.png");

    SDL_Surface *surfaceToggleON = IMG_Load("res/simple-toggle-on.png");
    SDL_Surface *surfaceToggleOFF = IMG_Load("res/simple-toggle-off.png");

    SDL_Surface *surfaceFileZIP = IMG_Load("res/file_zip.png");
    SDL_Surface *surfaceFile7Z = IMG_Load("res/file_7z.png");
    SDL_Surface *surfaceFileRAR = IMG_Load("res/file_rar.png");
    SDL_Rect rectPreviewIcon = {560 - surfaceFileZIP->w,
                                21 - surfaceFileZIP->h / 2};

    SDL_Surface *surfaceHasIcons = IMG_Load("res/themes_has_icons.png");
    SDL_Rect rectHasIcons = {560 - surfaceHasIcons->w,
                             21 - surfaceHasIcons->h / 2};
    SDL_Rect rectHasIconsPreviewIcon = {560 - surfaceFileZIP->w -
                                            surfaceHasIcons->w - 10,
                                        21 - surfaceFileZIP->h / 2};

    SDL_Rect preview_src_rect = {0, 0, 480, 360};
    SDL_Rect rectArrowLeft = {24, 210, 28, 32};
    SDL_Rect rectArrowRight = {588, 210, 28, 32};
    SDL_Rect rectPages = {559, 450};
    SDL_Rect rectThemeName = {20, 175, 600, 44};
    SDL_Rect rectThemePreview = {80, 46, 480, 360};
    SDL_Rect rectImageThemeNom = {77, 7, 557, 54};

    SDL_Surface *transparent_bg = SDL_CreateRGBSurface(0, 640, 480, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    SDL_FillRect(transparent_bg, NULL, 0xEE000000);

    SDL_Rect rectBottomBar = {0, 410};
    SDL_Surface *surfaceBottomBar = createBottomBar(font21);

    SDL_Surface *background_cache = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);

    int levelPage = 0;

    showCenteredMessage(video, screen, "Loading themes...", font30, color_white);

    char themes[NUMBER_OF_THEMES][STR_MAX];
    int installed_page = 0;
    int themes_count = listAllThemes(themes, installed_theme, &installed_page);
    int current_page = installed_page;

    showCenteredMessage(video, screen, "Loading previews...", font30, color_white);

    char preview_path[STR_MAX * 2];
    SDL_Surface *previews[themes_count];
    SDL_Surface *noPreview = IMG_Load("res/noThemePreview.png");

    for (int i = 0; i < themes_count; i++) {
        snprintf(preview_path, STR_MAX * 2 - 1, THEMES_DIR "/%s/preview.png", themes[i]);

        if (!is_file(preview_path))
            snprintf(preview_path, STR_MAX * 2 - 1, THEMES_DIR "/.previews/%s/preview.png", themes[i]);

        previews[i] = is_file(preview_path) ? IMG_Load(preview_path) : NULL;

        char loading_msg[STR_MAX];
        snprintf(loading_msg, STR_MAX - 1, "Loading previews... %d/%d", i + 1, themes_count);
        showCenteredMessage(video, screen, loading_msg, font30, color_white);
    }

    char cPages[25];

    SDL_Event event;
    Uint8 keystate[320] = {0};
    bool changed = true;

    Theme_s theme;
    char icon_pack_path[STR_MAX + 32];
    bool has_icons = false;
    bool apply_icons = false;
    bool is_preview = false;
    SDL_Surface *previewIcon = NULL;

    bool page_changed = true;
    bool render_dirty = true;

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            SDLKey key = event.key.keysym.sym;
            switch (event.type) {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYDOWN:
                keystate[key] = keystate[key] != 0 ? 2 : 1;
                changed = true;
                break;
            case SDL_KEYUP:
                keystate[key] = 0;
                changed = true;
                break;
            default:
                break;
            }
        }

        if (!changed)
            continue;

        if (keystate[SW_BTN_B]) {
            if (levelPage == 0)
                quit = true; // exit program
            else
                levelPage = 0;

            render_dirty = true;
        }

        if (keystate[SW_BTN_A]) {
            if (levelPage == 1) {
                showCenteredMessage(video, screen, "Installing...", font30, color_white);

                // Install theme
                installTheme(theme.path, apply_icons);
                printf_debug("Theme installed: %s\n", themes[current_page]);

                quit = true;
            }
            else {
                // Go to theme details/confirmation page
                levelPage = 1;
            }
            render_dirty = true;
        }

        if (levelPage == 1 && keystate[SW_BTN_X]) {
            apply_icons = !apply_icons;
            render_dirty = true;
        }

        if (levelPage == 0) {
            if (keystate[SW_BTN_RIGHT]) {
                if (current_page < themes_count - 1) {
                    current_page++;
                    page_changed = true;
                }
            }
            if (keystate[SW_BTN_LEFT]) {
                if (current_page > 0) {
                    current_page--;
                    page_changed = true;
                }
            }
        }

        if (quit)
            break;

        if (page_changed) {
            loadTheme(themes[current_page], &theme);
            snprintf(icon_pack_path, STR_MAX + 32 - 1, "%sicons", theme.path);
            apply_icons = true;
            has_icons = is_dir(icon_pack_path);

            if (strstr(theme.path, "/.previews/") != NULL) {
                is_preview = true;
                previewIcon = getPreviewIcon(theme.path, surfaceFileZIP, surfaceFile7Z, surfaceFileRAR);
            }
            else {
                is_preview = false;
            }

            page_changed = false;
            render_dirty = true;
        }

        if (!render_dirty)
            continue;

        if (levelPage == 0) {
            if (previews[current_page] == NULL) {
                SDL_BlitSurface(noPreview, NULL, screen, &rectThemePreview);
            }
            else {
                SDL_BlitSurface(previews[current_page], &preview_src_rect, screen, &rectThemePreview);
            }
            SDL_BlitSurface(background_page0, NULL, screen, NULL);

            if (current_page != 0)
                SDL_BlitSurface(surfaceArrowLeft, NULL, screen, &rectArrowLeft);
            if (current_page != themes_count - 1)
                SDL_BlitSurface(surfaceArrowRight, NULL, screen, &rectArrowRight);

            snprintf(cPages, sizeof(cPages) - 1, "%d/%d", current_page + 1, themes_count);
            imagePages = TTF_RenderUTF8_Blended(font30, cPages, color_white);
            rectPages.x = 620 - imagePages->w;
            rectPages.y = 450 - imagePages->h / 2;
            SDL_BlitSurface(imagePages, NULL, screen, &rectPages);
            SDL_FreeSurface(imagePages);

            char title[STR_MAX + 13];
            if (current_page == installed_page && !is_preview)
                snprintf(title, STR_MAX + 12, "%s - Installed", theme.name);
            else
                strcpy(title, theme.name);

            imageThemeNom = TTF_RenderUTF8_Blended(font21, title, color_white);
            SDL_BlitSurface(imageThemeNom, NULL, screen, &rectImageThemeNom);
            SDL_FreeSurface(imageThemeNom);

            if (has_icons) {
                SDL_BlitSurface(surfaceHasIcons, NULL, screen, &rectHasIcons);
            }

            if (is_preview) {
                SDL_BlitSurface(previewIcon, NULL, screen, has_icons ? &rectHasIconsPreviewIcon : &rectPreviewIcon);
            }

            SDL_BlitSurface(screen, NULL, background_cache, NULL);
        }
        else {
            SDL_BlitSurface(background_cache, NULL, screen, NULL);
            SDL_BlitSurface(transparent_bg, NULL, screen, NULL);
            rectThemeName.x = 20;
            rectThemeName.y = 175;

            imagePages = TTF_RenderUTF8_Blended(font40, theme.name, color_white);
            SDL_BlitSurface(imagePages, NULL, screen, &rectThemeName);
            SDL_FreeSurface(imagePages);
            rectThemeName.y += 50;

            if (strlen(theme.author) > 0) {
                char author[STR_MAX * 2];
                snprintf(author, STR_MAX * 2 - 1, "by %s", theme.author);
                imagePages = TTF_RenderUTF8_Blended(font30, author, color_white);
                SDL_BlitSurface(imagePages, NULL, screen, &rectThemeName);
                SDL_FreeSurface(imagePages);
                rectThemeName.y += 70;
            }

            SDL_BlitSurface(apply_icons ? surfaceToggleON : surfaceToggleOFF, NULL, screen, &rectThemeName);

            rectThemeName.x = 60;
            char msg[STR_MAX];
            sprintf(msg, "%s [%s]", has_icons ? "Apply icons" : "Reset icons", apply_icons ? "ON" : "OFF");
            imagePages = TTF_RenderUTF8_Blended(font21, msg, color_white);
            SDL_BlitSurface(imagePages, NULL, screen, &rectThemeName);
            SDL_FreeSurface(imagePages);

            SDL_BlitSurface(surfaceBottomBar, NULL, screen, &rectBottomBar);
        }

        SDL_BlitSurface(screen, NULL, video, NULL);
        SDL_Flip(video);

        changed = false;
        render_dirty = false;
    }

    msleep(100);

    for (int i = 0; i < themes_count; i++) {
        SDL_FreeSurface(previews[i]);
    }
    SDL_FreeSurface(noPreview);
    SDL_FreeSurface(surfaceArrowLeft);
    SDL_FreeSurface(surfaceArrowRight);
    SDL_FreeSurface(surfaceHasIcons);
    SDL_FreeSurface(background_page0);
    SDL_FreeSurface(transparent_bg);
    SDL_FreeSurface(surfaceBottomBar);
    SDL_FreeSurface(surfaceToggleON);
    SDL_FreeSurface(surfaceToggleOFF);
    SDL_FreeSurface(background_cache);
    SDL_FreeSurface(surfaceFileZIP);
    SDL_FreeSurface(surfaceFile7Z);
    SDL_FreeSurface(surfaceFileRAR);

    SDL_Quit();

    TTF_CloseFont(font40);
    TTF_CloseFont(font21);
    TTF_CloseFont(font30);

    return EXIT_SUCCESS;
}
