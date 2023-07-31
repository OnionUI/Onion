#include "sys/ioctl.h"
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <dirent.h>
#include <fcntl.h>
#include <libgen.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "system/keymap_hw.h"
#include "system/keymap_sw.h"
#include "utils/apply_icons.h"
#include "utils/file.h"
#include "utils/keystate.h"
#include "utils/log.h"
#include "utils/msleep.h"
#include "utils/str.h"
#include "utils/surfaceSetAlpha.h"

#define PACKAGE_DIR "/mnt/SDCARD/App/PackageManager/data/"

// Max number of records in the DB
#define LAYER_ITEM_COUNT 200
#define MAX_LAYER_NAME_SIZE 256
#define MAY_LAYER_DISPLAY 35

typedef struct package_s {
    char name[STR_MAX];
    bool installed;
    bool changed;
    bool complete;
} Package;

static char layer_names[][STR_MAX] = {"VERIFIED", "APPS", "EXPERT", "SUMMARY"};
static char layer_dirs[][STR_MAX] = {PACKAGE_DIR "Emu", PACKAGE_DIR "App",
                                     PACKAGE_DIR "RApp", ""};
static const int tab_count = 4;
static const int summary_tab = tab_count - 1;

static Package packages[4][LAYER_ITEM_COUNT];
static int package_count[] = {0, 0, 0, 0};
static int package_installed_count[] = {0, 0, 0, 0};
static int nSelection = 0;
static int nListPosition = 0;
static int nTab = 0;
static int changes_installs[] = {0, 0, 0, 0};
static int changes_removals[] = {0, 0, 0, 0};

static SDL_Surface *video = NULL, *screen = NULL, *surfaceBackground = NULL,
                   *surfaceSelection = NULL, *surfaceTableau = NULL,
                   *surfaceScroller = NULL, *surfaceMarker = NULL,
                   *surfaceCheck = NULL, *surfaceCross = NULL,
                   *surfaceDotActive = NULL, *surfaceDotNeutral = NULL,
                   *surfaceDotApplyActive = NULL,
                   *surfaceDotApplyNeutral = NULL;

static SDL_Rect rectSummaryPos = {30, 85};
static SDL_Surface *surfaceSummary = NULL;
static int summaryScrollY = 0;
static int scrollSpeed = 28;

static TTF_Font *font18 = NULL;
static TTF_Font *font25 = NULL;
static TTF_Font *font35 = NULL;

static SDL_Color color_white = {255, 255, 255};
static SDL_Color color_red = {247, 145, 145};
static SDL_Color color_green = {154, 247, 145};

int changesInstalls(void)
{
    int total = 0;
    for (int i = 0; i < tab_count; i++)
        total += changes_installs[i];
    return total;
}

int changesRemovals(void)
{
    int total = 0;
    for (int i = 0; i < tab_count; i++)
        total += changes_removals[i];
    return total;
}

int changesTotal(void) { return changesInstalls() + changesRemovals(); }

int totalInstalls(void)
{
    int total = 0;
    for (int i = 0; i < tab_count; i++)
        total += package_installed_count[i];
    return total;
}

void setLayersInstall(bool should_install, int layer)
{
    for (int i = 0; i < package_count[layer]; i++) {
        Package *package = &packages[layer][i];
        bool new_value = package->installed ? !should_install : should_install;

        if (package->changed != new_value) {
            package->changed = new_value;

            if (package->installed) {
                changes_removals[layer] += new_value ? 1 : -1;
                if (!package->complete)
                    changes_installs[layer] += new_value ? -1 : 1;
            }
            else
                changes_installs[layer] += new_value ? 1 : -1;
        }
    }
}

void layerToggleAll(int layer)
{
    bool all_on = true;

    for (int i = 0; i < package_count[layer]; i++) {
        Package *package = &packages[layer][i];
        if (package->installed == package->changed) {
            all_on = false;
            break;
        }
    }

    setLayersInstall(!all_on, layer);
}

void layerReset(int layer)
{
    for (int i = 0; i < package_count[layer]; i++) {
        Package *package = &packages[layer][i];

        if (package->changed) {
            if (package->installed) {
                changes_removals[layer]--;
                if (!package->complete)
                    changes_installs[layer]++;
            }
            else
                changes_installs[layer]--;

            package->changed = false;
        }
    }
}

void appUninstall(char *basePath, int strlenBase)
{
    char path[1000];
    char pathInstalledApp[1000];

    struct dirent *dp;
    DIR *dir = opendir(basePath);

    // Unable to open directory stream
    if (!dir)
        return;

    while ((dp = readdir(dir)) != NULL) {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
            // Construct new path from our base path
            sprintf(path, "%s/%s", basePath, dp->d_name);

            if (exists(path)) {
                sprintf(pathInstalledApp, "/mnt/SDCARD%s", path + strlenBase);

                if (is_file(pathInstalledApp))
                    remove(pathInstalledApp);

                if (is_dir(path))
                    appUninstall(path, strlenBase);

                if (is_dir(pathInstalledApp))
                    rmdir(pathInstalledApp);
            }
        }
    }

    closedir(dir);
}

bool checkAppInstalled(const char *basePath, int base_len, int level,
                       bool complete)
{
    char path[1000];
    char pathInstalledApp[1000];

    struct dirent *dp;
    DIR *dir = opendir(basePath);

    int run = 1;
    bool is_installed = true;

    // Unable to open directory stream
    if (!dir)
        return true;

    while ((dp = readdir(dir)) != NULL && run == 1) {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
            continue;

        // Ignore other dirs
        if ((level == 0 && strcmp(dp->d_name, "Emu") > 0 &&
             strcmp(dp->d_name, "App") > 0 && strcmp(dp->d_name, "RApp") > 0) ||
            (level == 1 && strcmp(dp->d_name, "romscripts") == 0))
            continue;

        // Construct new path from our base path
        sprintf(path, "%s/%s", basePath, dp->d_name);

        if (exists(path)) {
            sprintf(pathInstalledApp, "/mnt/SDCARD%s", path + base_len);

            if (!exists(pathInstalledApp))
                is_installed = false;
            else if (dp->d_type == DT_DIR)
                is_installed =
                    checkAppInstalled(path, base_len, level + 1, complete);

            if (!complete && level >= 2 && exists(pathInstalledApp))
                return true;

            if ((complete || level < 2) && !is_installed)
                run = 0;
        }
    }

    closedir(dir);
    return is_installed;
}

void loadResources(bool auto_update)
{
    DIR *dp;
    struct dirent *ep;
    char basePath[1000];

    for (int nT = 0; nT < tab_count; nT++) {
        const char *data_path = layer_dirs[nT];
        package_count[nT] = 0;

        if (strlen(data_path) == 0 || !exists(data_path) ||
            (dp = opendir(data_path)) == NULL)
            continue;

        while ((ep = readdir(dp)) && package_count[nT] < LAYER_ITEM_COUNT) {
            char cShort[MAX_LAYER_NAME_SIZE];
            strcpy(cShort, ep->d_name);

            const char *file_name = ep->d_name;
            if (file_name[0] != '.') {
                // Installation check
                sprintf(basePath, "%s/%s", data_path, file_name);

                bool is_installed =
                    checkAppInstalled(basePath, strlen(basePath), 0, false);
                bool is_complete =
                    !auto_update && is_installed
                        ? checkAppInstalled(basePath, strlen(basePath), 0, true)
                        : false;

                Package package = {.installed = is_installed,
                                   .changed = false,
                                   .complete = is_complete};

                if (is_installed) {
                    package_installed_count[nT]++;

                    if (!is_complete)
                        changes_installs[nT]++;
                }

                strcpy(package.name, file_name);

                packages[nT][package_count[nT]] = package;
                package_count[nT]++;
            }
        }

        closedir(dp);
    }

    Package temp;

    // Sort function
    for (int nT = 0; nT < tab_count; nT++) {
        bool found = true;

        while (found) {
            found = false;

            for (int i = 0; i < package_count[nT] - 1; i++) {
                Package *package_a = &packages[nT][i];
                Package *package_b = &packages[nT][i + 1];

                if (strcmp(package_a->name, package_b->name) > 0) {
                    temp = *package_a;
                    *package_a = *package_b;
                    *package_b = temp;
                    found = true;
                }
            }
        }
    }
}

SDL_Surface *createLabelSurface(Package *package)
{
    SDL_Surface *textbox =
        SDL_CreateRGBSurface(0, 593, 49, 32, 0x00FF0000, 0x0000FF00, 0x000000FF,
                             0xFF000000); /* important */
    SDL_FillRect(textbox, NULL, 0x000000FF);

    char label_text[STR_MAX], parens[STR_MAX] = "";
    strncpy(label_text, package->name, STR_MAX - 1);

    if (strchr(package->name, '(') != NULL) {
        parens[0] = '(';
        strncat(parens, str_split(label_text, "("), STR_MAX - 2);
    }

    SDL_Surface *label_surface =
        TTF_RenderUTF8_Blended(font25, label_text, color_white);
    SDL_SetAlpha(label_surface, 0, 0); /* important */
    SDL_Rect label_pos = {0, 0};
    SDL_BlitSurface(label_surface, NULL, textbox, &label_pos);

    if (package->installed && !package->complete)
        strcat(parens, "**");
    else if (package->changed)
        strcat(parens, "*");

    if (strlen(parens) > 0) {
        SDL_Surface *parens_surface =
            TTF_RenderUTF8_Blended(font25, parens, color_white);
        SDL_SetAlpha(parens_surface, 0, 0); /* important */
        surfaceSetAlpha(parens_surface, 120);
        SDL_Rect parens_pos = {label_surface->w, 0};
        SDL_BlitSurface(parens_surface, NULL, textbox, &parens_pos);
        SDL_FreeSurface(parens_surface);
    }

    SDL_FreeSurface(label_surface);

    return textbox;
}

void displayLayersNames(void)
{
    SDL_Rect rectResName = {30, 92, 80, 20};
    SDL_Surface *surfaceResName;
    for (int i = 0; i < 7; i++) {
        if ((i + nListPosition) < package_count[nTab]) {
            Package *package = &packages[nTab][i + nListPosition];
            surfaceResName = createLabelSurface(package);
            rectResName.y = 90 + i * 47;
            SDL_BlitSurface(surfaceResName, NULL, screen, &rectResName);
            SDL_FreeSurface(surfaceResName);
        }
    }
}

void renderFooter(const char *footer_str)
{
    SDL_Surface *footer =
        TTF_RenderUTF8_Blended(font18, footer_str, color_white);
    surfaceSetAlpha(footer, 120);
    SDL_Rect footer_rect = {320 - footer->w / 2, 414};
    SDL_BlitSurface(footer, NULL, screen, &footer_rect);
    SDL_FreeSurface(footer);
}

void displayLayersInstall(void)
{
    SDL_Rect rectInstall = {600 - surfaceCheck->w, 96};

    for (int i = 0; i < 7; i++) {
        if ((i + nListPosition) < package_count[nTab]) {
            Package *package = &packages[nTab][i + nListPosition];
            rectInstall.y = 107 - surfaceCheck->h / 2 + i * 47;
            if (package->installed != package->changed)
                SDL_BlitSurface(surfaceCheck, NULL, screen, &rectInstall);
            else
                SDL_BlitSurface(surfaceCross, NULL, screen, &rectInstall);
        }
    }

    char footer_str[STR_MAX];
    sprintf(footer_str, "%d added  |  %d removed  |  %d installed  |  %d total",
            changes_installs[nTab], changes_removals[nTab],
            package_installed_count[nTab], package_count[nTab]);
    renderFooter(footer_str);
}

void showScroller(void)
{
    SDL_Rect rectMarker = {614, 87};
    SDL_Rect rectScroller = {611, 87};
    int scrollerHeight = 323;

    if (nTab == summary_tab) {
        if (surfaceSummary == NULL || surfaceSummary->h <= 323)
            return;

        scrollerHeight = round(323.0 * 323.0 / (float)(surfaceSummary->h));
        float scroll = (float)summaryScrollY / (float)(surfaceSummary->h - 323);
        rectScroller.y += round(scroll * (float)(323 - scrollerHeight));
    }
    else if (package_count[nTab] - 7 > 0) {
        scrollerHeight = round(323.0 * 7.0 / (float)package_count[nTab]);

        if (scrollerHeight < 10)
            scrollerHeight = 10;

        float scroll = (float)nListPosition / (float)(package_count[nTab] - 7);
        rectScroller.y += round(scroll * (float)(323 - scrollerHeight));
    }

    float markerSlice = 323.0 / (float)(package_count[nTab]);
    float markerOffset = 0.5 * markerSlice - (float)surfaceMarker->h / 2;

    for (int i = 0; i < package_count[nTab]; i++) {
        Package *package = &packages[nTab][i];

        if (package->installed == package->changed)
            continue;

        rectMarker.y = 87 + round((float)i * markerSlice + markerOffset);
        SDL_BlitSurface(surfaceMarker, NULL, screen, &rectMarker);
    }

    static SDL_Rect rectScrollerTop = {0, 0, 10, 5};
    static SDL_Rect rectScrollerMiddle = {0, 5, 10, 10};
    static SDL_Rect rectScrollerBottom = {0, 15, 10, 5};

    SDL_BlitSurface(surfaceScroller, &rectScrollerTop, screen, &rectScroller);
    rectScroller.y += 5;

    int scrollerMiddle = scrollerHeight - 10;

    while (scrollerMiddle > 0) {
        int h = scrollerMiddle > 10 ? 10 : scrollerMiddle;
        rectScrollerMiddle.h = h;
        SDL_BlitSurface(surfaceScroller, &rectScrollerMiddle, screen,
                        &rectScroller);
        rectScroller.y += h;
        scrollerMiddle -= h;
    }

    SDL_BlitSurface(surfaceScroller, &rectScrollerBottom, screen,
                    &rectScroller);
}

bool confirmDoNothing(KeyState *keystate)
{
    bool quit = false;
    SDL_Surface *image = IMG_Load("res/confirmDoNothing.png");

    SDL_BlitSurface(image, NULL, screen, NULL);
    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);

    bool confirm = false;

    while (!quit) {
        if (updateKeystate(keystate, &quit, true, NULL)) {
            if (keystate[SW_BTN_A] == PRESSED)
                confirm = true;
            else if (keystate[SW_BTN_A] == RELEASED && confirm)
                quit = true;
            else if (keystate[SW_BTN_B] == PRESSED)
                quit = true;
        }
    }

    SDL_FreeSurface(image);

    return confirm;
}

typedef enum Alignment { ALIGN_START,
                         ALIGN_CENTER,
                         ALIGN_END } alignment_e;

int alignCoord(int n, int size, alignment_e alignment)
{
    switch (alignment) {
    case ALIGN_START:
        return n;
    case ALIGN_CENTER:
        return n - size / 2;
    case ALIGN_END:
        return n - size;
    default:
        return n;
    }
}

void renderTabName(const char *name, int x, alignment_e alignment, bool active,
                   bool has_changes)
{
    char name_str[STR_MAX];
    snprintf(name_str, STR_MAX - 1, has_changes ? "%s*" : "%s", name);

    SDL_Surface *tab_name =
        TTF_RenderUTF8_Blended(active ? font25 : font18, name_str, color_white);
    SDL_Rect tab_rect = {alignCoord(x, tab_name->w, alignment),
                         (active ? 53 : 56) - 2 - tab_name->h / 2};

    SDL_BlitSurface(tab_name, NULL, screen, &tab_rect);
    SDL_FreeSurface(tab_name);
}

void renderCurrentTab(void)
{
    int lTab = nTab == 0 ? tab_count - 1 : nTab - 1,
        rTab = nTab == tab_count - 1 ? 0 : nTab + 1;

    renderTabName(layer_names[lTab], 50, ALIGN_START, false,
                  changes_installs[lTab] > 0 || changes_removals[lTab] > 0);
    renderTabName(layer_names[rTab], 590, ALIGN_END, false,
                  changes_installs[rTab] > 0 || changes_removals[rTab] > 0);
    renderTabName(layer_names[nTab], 320, ALIGN_CENTER, true,
                  changes_installs[nTab] > 0 || changes_removals[nTab] > 0);

    int tab_dots_width =
        (tab_count - 1) * surfaceDotNeutral->w + surfaceDotActive->w;
    SDL_Rect rectTabDot = {320 - tab_dots_width / 2, 14};

    for (int i = 0; i < tab_count; i++) {
        SDL_Surface *current_dot;

        if (i != summary_tab)
            current_dot = i == nTab ? surfaceDotActive : surfaceDotNeutral;
        else
            current_dot =
                i == nTab ? surfaceDotApplyActive : surfaceDotApplyNeutral;

        rectTabDot.y = 14 - current_dot->h / 2;

        SDL_BlitSurface(current_dot, NULL, screen, &rectTabDot);
        rectTabDot.x += current_dot->w;
    }
}

int renderSummaryLine(SDL_Surface *surfaceTemp, int pos_y, const char *line_str,
                      int alpha, SDL_Color color)
{
    SDL_Surface *surfaceLine = TTF_RenderUTF8_Blended(font18, line_str, color);
    SDL_Rect rectLine = {0, pos_y};
    int h = surfaceLine->h;

    SDL_SetAlpha(surfaceLine, 0, 0); /* important */
    surfaceSetAlpha(surfaceLine, alpha);

    SDL_BlitSurface(surfaceLine, NULL, surfaceTemp, &rectLine);
    SDL_FreeSurface(surfaceLine);

    return h;
}

void renderSummary()
{
    SDL_Rect rectSummaryFrame = {0, summaryScrollY, 593, 327};

    if (surfaceSummary == NULL) {
        SDL_Surface *surfaceTemp =
            SDL_CreateRGBSurface(0, 593, 4096, 32, 0x00FF0000, 0x0000FF00,
                                 0x000000FF, 0xFF000000); /* important */
        SDL_SetAlpha(surfaceTemp, 0, 0);                  /* important */

        int pos_y = 0;

        for (int nT = 0; nT < tab_count; nT++) {
            const char *data_path = layer_dirs[nT];

            if (strlen(data_path) == 0 || !exists(data_path))
                continue;

            if (changes_installs[nT] + changes_removals[nT] == 0)
                continue;

            pos_y += 10;

            char line_str[STR_MAX * 2];
            sprintf(line_str, "%s:", layer_names[nT]);

            if (changes_installs[nT] > 0)
                sprintf(line_str + strlen(line_str), " %d added",
                        changes_installs[nT]);

            if (changes_removals[nT] > 0) {
                int len = strlen(line_str);
                if (changes_installs[nT] > 0) {
                    strcpy(line_str + len, ",");
                    len += 1;
                }
                sprintf(line_str + len, " %d removed", changes_removals[nT]);
            }

            pos_y += renderSummaryLine(surfaceTemp, pos_y, line_str, 255,
                                       color_white) +
                     5;

            for (int i = 0; i < package_count[nT]; i++) {
                Package *package = &packages[nT][i];

                if (!package->changed &&
                    (!package->installed || package->complete))
                    continue;

                memset(line_str, 0, STR_MAX * 2);

                bool is_removed = package->changed && package->installed;
                sprintf(line_str, "[%s]  %s", is_removed ? "−" : "+",
                        package->name);

                pos_y +=
                    renderSummaryLine(surfaceTemp, pos_y, line_str, 120,
                                      is_removed ? color_red : color_green) +
                    5;
            }
        }

        surfaceSummary =
            SDL_CreateRGBSurface(0, 593, pos_y, 32, 0x00FF0000, 0x0000FF00,
                                 0x000000FF, 0xFF000000); /* important */
        SDL_FillRect(surfaceSummary, NULL, 0x000000FF);
        SDL_BlitSurface(surfaceTemp, NULL, surfaceSummary, NULL);
    }

    SDL_BlitSurface(surfaceSummary, &rectSummaryFrame, screen, &rectSummaryPos);
}

void refreshSummary()
{
    if (surfaceSummary != NULL)
        SDL_FreeSurface(surfaceSummary);
    surfaceSummary = NULL;
    summaryScrollY = 0;
}

bool summaryScrollBy(int offset)
{
    bool changed = false;
    int y_max = surfaceSummary->h - 323;

    if ((offset > 0 && summaryScrollY < y_max) ||
        (offset < 0 && summaryScrollY > 0)) {
        summaryScrollY += offset;
        changed = true;
    }

    if (summaryScrollY > y_max)
        summaryScrollY = y_max;
    else if (summaryScrollY < 0)
        summaryScrollY = 0;

    return changed;
}

bool getPackageMainPath(char *out_path, const char *data_path,
                        const char *package_name)
{
    const char *base_dir = basename((char *)data_path);
    sprintf(out_path, "%s/%s/%s/", data_path, package_name, base_dir);

    if (!is_dir(out_path))
        return false;

    struct dirent *dp;
    DIR *dir = opendir(out_path);

    // Unable to open directory stream
    if (!dir)
        return false;

    while ((dp = readdir(dir)) != NULL) {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
            continue;
        if (dp->d_type != DT_DIR)
            continue;
        sprintf(out_path, "/mnt/SDCARD/%s/%s", base_dir, dp->d_name);
        return is_dir(out_path);
    }

    return false;
}

void callPackageInstaller(const char *data_path, const char *package_name,
                          bool install)
{
    char main_path[STR_MAX], cmd[STR_MAX];

    if (getPackageMainPath(main_path, data_path, package_name)) {
        char config_path[STR_MAX + 32];
        snprintf(config_path, STR_MAX + 32 - 1, "%s/config.json", main_path);

        if (install && is_file(config_path))
            apply_singleIcon(config_path);

        char installer_path[STR_MAX + 32];
        concat(installer_path, main_path,
               install ? "/install.sh" : "/uninstall.sh");
        if (is_file(installer_path)) {
            sprintf(cmd,
                    install
                        ? "cd \"%s\"; chmod a+x ./install.sh; ./install.sh"
                        : "cd \"%s\"; chmod a+x ./uninstall.sh; ./uninstall.sh",
                    main_path);
            system(cmd);
        }
    }
}

int main(int argc, char *argv[])
{
    bool show_confirm = false;
    bool auto_update = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--confirm") == 0)
            show_confirm = true;
        else if (strcmp(argv[i], "--auto_update") == 0)
            auto_update = true;
        else {
            printf("unknown argument: %s\n", argv[i]);
            return EXIT_FAILURE;
        }
    }

    SDL_Init(SDL_INIT_VIDEO);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_EnableKeyRepeat(300, 50);
    TTF_Init();

    video = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE);
    screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);

    surfaceBackground = IMG_Load("res/bgApp.png");
    surfaceSelection = IMG_Load("res/selection.png");
    surfaceTableau = IMG_Load("res/tableau.png");
    surfaceScroller = IMG_Load("res/scroller.png");
    surfaceMarker = IMG_Load("res/marker.png");
    surfaceDotActive = IMG_Load("res/dot-a.png");
    surfaceDotNeutral = IMG_Load("res/dot-n.png");
    surfaceDotApplyActive = IMG_Load("res/dot-apply-a.png");
    surfaceDotApplyNeutral = IMG_Load("res/dot-apply-n.png");
    surfaceCheck = IMG_Load("/mnt/SDCARD/.tmp_update/res/toggle-on.png");
    surfaceCross = IMG_Load("/mnt/SDCARD/.tmp_update/res/toggle-off.png");

    font18 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 18);
    font25 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 25);
    font35 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 35);

    if (!auto_update) {
        SDL_Surface *loadingScreen = IMG_Load("res/loading.png");
        SDL_BlitSurface(loadingScreen, NULL, screen, NULL);
        SDL_BlitSurface(screen, NULL, video, NULL);
        SDL_Flip(video);
        SDL_FreeSurface(loadingScreen);
    }

    loadResources(auto_update);

    SDL_Rect rectSelection = {14, 84, 593, 49};

    bool quit = false;
    bool state_changed = true;
    KeyState keystate[320] = {0};

    bool apply_changes = false;

    bool show_reinstall = auto_update && totalInstalls() > 0;
    SDL_Surface *reinstall_surface = IMG_Load("res/start_alt.png");

    if (auto_update) {
        apply_singleIcon("/mnt/SDCARD/App/PackageManager/config.json");
        quit = true;
        apply_changes = true;
    }

    while (!quit) {
        if (updateKeystate(keystate, &quit, true, NULL)) {
            if (keystate[SW_BTN_R1] >= PRESSED) {
                if (nTab < tab_count - 1)
                    nTab++;
                else
                    nTab = 0;
                nSelection = 0;
                nListPosition = 0;
                state_changed = true;
            }
            if (keystate[SW_BTN_L1] >= PRESSED) {
                if (nTab > 0)
                    nTab--;
                else
                    nTab = tab_count - 1;
                nSelection = 0;
                nListPosition = 0;
                state_changed = true;
            }

            if (nTab == summary_tab && surfaceSummary != NULL) {
                if (keystate[SW_BTN_R2] >= PRESSED) {
                    if (summaryScrollBy(323))
                        state_changed = true;
                }

                if (keystate[SW_BTN_L2] >= PRESSED) {
                    if (summaryScrollBy(-323))
                        state_changed = true;
                }

                if (keystate[SW_BTN_DOWN] >= PRESSED) {
                    if (summaryScrollBy(scrollSpeed))
                        state_changed = true;
                }

                if (keystate[SW_BTN_UP] >= PRESSED) {
                    if (summaryScrollBy(-scrollSpeed))
                        state_changed = true;
                }
            }
            else if (package_count[nTab] > 0) {
                if (keystate[SW_BTN_R2] >= PRESSED) {
                    if ((nListPosition + 14) < package_count[nTab]) {
                        nListPosition += 7;
                    }
                    else if ((nListPosition + 7) < package_count[nTab]) {
                        nListPosition = package_count[nTab] - 7;
                        nSelection = 6;
                    }
                    state_changed = true;
                }

                if (keystate[SW_BTN_L2] >= PRESSED) {
                    if ((nListPosition - 7) > 0) {
                        nListPosition -= 7;
                    }
                    else {
                        nListPosition = 0;
                        nSelection = 0;
                    }
                    state_changed = true;
                }

                if (keystate[SW_BTN_DOWN] >= PRESSED) {
                    if (nSelection < 6) {
                        nSelection++;
                    }
                    else if (nSelection + nListPosition <
                             package_count[nTab] - 1) {
                        nListPosition++;
                    }
                    else if (keystate[SW_BTN_DOWN] == PRESSED &&
                             nSelection + nListPosition >=
                                 package_count[nTab] - 1) {
                        nSelection = nListPosition = 0;
                    }
                    state_changed = true;
                }

                if (keystate[SW_BTN_UP] >= PRESSED) {
                    if (nSelection > 0) {
                        nSelection--;
                    }
                    else if (nListPosition > 0) {
                        nListPosition--;
                    }
                    else if (keystate[SW_BTN_UP] == PRESSED) {
                        nSelection = 6;
                        nListPosition = package_count[nTab] - 7;
                    }
                    state_changed = true;
                }
            }

            bool back_pressed = keystate[SW_BTN_B] == PRESSED;
            bool apply_pressed =
                keystate[SW_BTN_START] == PRESSED ||
                (keystate[SW_BTN_A] == PRESSED && nTab == summary_tab);

            if (back_pressed || apply_pressed) {
                if (apply_pressed && nTab != summary_tab) {
                    nTab = summary_tab;
                    nSelection = 0;
                    nListPosition = 0;
                }
                else {
                    if (apply_pressed)
                        apply_changes = true;

                    if (show_confirm) {
                        if (apply_changes) {
                            if (changesTotal() > 0 ||
                                (auto_update &&
                                 package_installed_count[0] > 0) ||
                                confirmDoNothing(keystate))
                                quit = true;
                        }
                        else if (confirmDoNothing(keystate))
                            quit = true;
                    }
                    else
                        quit = true;
                }
                state_changed = true;
            }

            if (package_count[nTab] > 0) {
                if (keystate[SW_BTN_A] == PRESSED ||
                    keystate[SW_BTN_LEFT] == PRESSED ||
                    keystate[SW_BTN_RIGHT] == PRESSED) {
                    int pos = nListPosition + nSelection;
                    if (pos < package_count[nTab]) {
                        Package *package = &packages[nTab][pos];
                        bool prev_value = package->changed;

                        if (keystate[SW_BTN_A] == PRESSED)
                            package->changed = !package->changed;
                        else if (keystate[SW_BTN_LEFT] == PRESSED)
                            package->changed = package->installed;
                        else if (keystate[SW_BTN_RIGHT] == PRESSED)
                            package->changed = !package->installed;

                        if (package->changed != prev_value) {
                            if (package->installed) {
                                changes_removals[nTab] +=
                                    package->changed ? 1 : -1;
                                if (!package->complete)
                                    changes_installs[nTab] +=
                                        package->changed ? -1 : 1;
                            }
                            else {
                                changes_installs[nTab] +=
                                    package->changed ? 1 : -1;
                            }
                            refreshSummary();
                            state_changed = true;
                        }
                    }
                }

                if (keystate[SW_BTN_X] == PRESSED) {
                    layerToggleAll(nTab);
                    refreshSummary();
                    state_changed = true;
                }

                if (keystate[SW_BTN_Y] == PRESSED) {
                    layerReset(nTab);
                    refreshSummary();
                    state_changed = true;
                }
            }
        }

        if (quit)
            break;

        if (state_changed) {
            rectSelection.y = 83 + nSelection * 47;

            SDL_FillRect(screen, NULL, 0);
            SDL_BlitSurface(surfaceBackground, NULL, screen, NULL);
            SDL_BlitSurface(surfaceTableau, NULL, screen, NULL);

            int changes_total = changesTotal();

            if (changes_total > 0) {
                int installs_count = changesInstalls();
                int removals_count = changesRemovals();
                char status_str[STR_MAX] = "";

                if (installs_count > 0)
                    sprintf(status_str, "+%d", installs_count);

                if (removals_count > 0) {
                    int len = strlen(status_str);
                    if (len > 0) {
                        strcpy(status_str + len, "  ");
                        len += 2;
                    }
                    sprintf(status_str + len, " −%d", removals_count);
                }

                SDL_Surface *status =
                    TTF_RenderUTF8_Blended(font18, status_str, color_white);
                SDL_Rect status_rect = {620 - status->w, 16 - status->h / 2};
                SDL_BlitSurface(status, NULL, screen, &status_rect);
                SDL_FreeSurface(status);
            }

            renderCurrentTab();

            if (nTab == summary_tab) {
                if (changes_total > 0) {
                    renderSummary();
                    showScroller();
                    renderFooter("Press A or START to apply changes");
                }
                else {
                    SDL_Surface *status = TTF_RenderUTF8_Blended(
                        font35, "NO CHANGES", color_white);
                    SDL_Rect status_rect = {
                        alignCoord(320, status->w, ALIGN_CENTER),
                        alignCoord(247, status->h, ALIGN_CENTER)};
                    SDL_BlitSurface(status, NULL, screen, &status_rect);
                    SDL_FreeSurface(status);
                    renderFooter("Press A or START to exit");
                }
            }
            else if (package_count[nTab] > 0) {
                if (surfaceSelection == NULL)
                    SDL_FillRect(screen, &rectSelection, 0xFF333333);
                else
                    SDL_BlitSurface(surfaceSelection, NULL, screen,
                                    &rectSelection);

                displayLayersNames();
                showScroller();
                displayLayersInstall();
            }

            if (show_reinstall) {
                SDL_Rect reinstall_rect = {640 - reinstall_surface->w,
                                           480 - reinstall_surface->h};
                SDL_BlitSurface(reinstall_surface, NULL, screen,
                                &reinstall_rect);
            }

            SDL_BlitSurface(screen, NULL, video, NULL);
            SDL_Flip(video);

            state_changed = false;
        }
    }

    if (apply_changes) {
        // installation
        char cmd[STR_MAX * 2 + 100];

        SDL_Surface *surfaceBackground = IMG_Load("/mnt/SDCARD/.tmp_update/res/waitingBG.png");
        SDL_Surface *surfaceMessage;

        for (int nT = 0; nT < tab_count; nT++) {
            const char *data_path = layer_dirs[nT];

            if (strlen(data_path) == 0 || !exists(data_path))
                continue;

            SDL_Rect rectMessage = {10, 420, 603, 48};

            for (int nLayer = 0; nLayer < package_count[nT]; nLayer++) {
                Package *package = &packages[nT][nLayer];

                bool should_apply =
                    auto_update || (package->installed && !package->complete) ||
                    package->changed;
                bool should_install = package->installed != package->changed ||
                                      (package->installed &&
                                       !package->complete && !package->changed);

                if (!should_apply)
                    continue;

                if (should_install) {
                    printf_debug("Installing %s...\n", package->name);
                    SDL_BlitSurface(surfaceBackground, NULL, screen, NULL);

                    surfaceMessage = TTF_RenderUTF8_Blended(
                        font35, package->name, color_white);
                    SDL_BlitSurface(surfaceMessage, NULL, screen, &rectMessage);
                    SDL_FreeSurface(surfaceMessage);

                    SDL_BlitSurface(screen, NULL, video, NULL);
                    SDL_Flip(video);

                    sprintf(cmd,
                            "/mnt/SDCARD/.tmp_update/script/pacman_install.sh \"%s\" \"%s\"",
                            data_path, package->name);
                    system(cmd);
                    sync();

                    callPackageInstaller(data_path, package->name, true);
                }
                else if (package->installed) {
                    printf_debug("Removing %s...\n", package->name);
                    callPackageInstaller(data_path, package->name, false);

                    // app removal
                    char pathAppUninstall[1000];
                    sprintf(pathAppUninstall, "%s/%s", data_path, package->name);
                    appUninstall(pathAppUninstall, strlen(pathAppUninstall));
                }
            }
        }
    }

#ifndef PLATFORM_MIYOOMINI
    msleep(200);
#endif

    TTF_CloseFont(font18);
    TTF_CloseFont(font25);
    TTF_CloseFont(font35);
    TTF_Quit();
    SDL_FreeSurface(reinstall_surface);
    SDL_FreeSurface(surfaceCheck);
    SDL_FreeSurface(surfaceCross);
    SDL_FreeSurface(surfaceBackground);
    SDL_FreeSurface(surfaceTableau);
    SDL_FreeSurface(surfaceSelection);
    SDL_FreeSurface(surfaceScroller);
    SDL_FreeSurface(surfaceMarker);
    SDL_FreeSurface(surfaceDotActive);
    SDL_FreeSurface(surfaceDotNeutral);
    SDL_FreeSurface(surfaceDotApplyActive);
    SDL_FreeSurface(surfaceDotApplyNeutral);
    SDL_Quit();

    return EXIT_SUCCESS;
}
