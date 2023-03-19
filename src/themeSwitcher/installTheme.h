#ifndef THEME_SWITCHER_INSTALL_THEME_H__
#define THEME_SWITCHER_INSTALL_THEME_H__

#include <dirent.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

#include "theme/config.h"
#include "utils/apply_icons.h"
#include "utils/file.h"
#include "utils/json.h"
#include "utils/log.h"
#include "utils/str.h"

#ifdef PLATFORM_MIYOOMINI
#define SCRIPT_DIR "/mnt/SDCARD/.tmp_update/script"
#else
#define SCRIPT_DIR "./script"
#endif

#define SYSTEM_SKIN_DIR "/mnt/SDCARD/miyoo/app/skin"
#define THEMES_DIR "/mnt/SDCARD/Themes"

#ifndef DT_DIR
#define DT_DIR 4
#endif

// Max number of records in the DB
#define NUMBER_OF_THEMES 500

int _comp_themes(const void *a, const void *b)
{
    return strcasecmp((const char *)a, (const char *)b);
}

void loadThemeDirectory(const char *theme_dir,
                        char themes_out[NUMBER_OF_THEMES][STR_MAX], int *count,
                        bool check_preview)
{
    DIR *dp;
    struct dirent *ep;
    char config_path[STR_MAX * 2];
    char preview_path[STR_MAX * 2];

    if ((dp = opendir(theme_dir)) != NULL) {
        while ((ep = readdir(dp))) {
            if (ep->d_type != DT_DIR)
                continue;
            if (ep->d_name[0] == '.')
                continue;

            snprintf(config_path, STR_MAX * 2 - 1, "%s/%s/config.json",
                     theme_dir, ep->d_name);

            if (check_preview) {
                snprintf(preview_path, STR_MAX * 2 - 1,
                         THEMES_DIR "/.previews/%s/config.json", ep->d_name);

                if (is_file(preview_path))
                    continue;
            }

            if (is_file(config_path)) {
                strcpy(themes_out[*count], ep->d_name);
                *count += 1;
            }
        }
        closedir(dp);
    }
    else {
        perror("Couldn't open the Themes directory");
    }
}

int listAllThemes(char themes_out[NUMBER_OF_THEMES][STR_MAX],
                  const char *installed_theme, int *installed_page)
{
    int count = 0;

    system(SCRIPT_DIR "/themes_extract_previews.sh");
    sync();

    loadThemeDirectory(THEMES_DIR, themes_out, &count, true);
    loadThemeDirectory(THEMES_DIR "/.previews", themes_out, &count, false);

    qsort(themes_out, count, sizeof(char) * STR_MAX, _comp_themes);

    for (int i = 0; i < count; i++) {
        if (strcmp(themes_out[i], installed_theme) == 0) {
            *installed_page = i;
            break;
        }
    }

    return count;
}

bool checkPreview(const char *preview_path)
{
    if (!is_dir(preview_path))
        return false;

    char source_path[STR_MAX * 2];
    snprintf(source_path, STR_MAX * 2 - 1, "%s/source", preview_path);

    if (!is_file(source_path))
        return false;

    FILE *fp;
    char archive_path[STR_MAX * 2];
    file_get(fp, source_path, "%[^\n]", archive_path);

    if (!is_file(archive_path))
        return false;

    return true;
}

void loadTheme(const char *theme_name, Theme_s *theme_out)
{
    char theme_path[STR_MAX * 2];
    snprintf(theme_path, STR_MAX * 2 - 1, THEMES_DIR "/.previews/%s/",
             theme_name);

    if (!checkPreview(theme_path))
        snprintf(theme_path, STR_MAX * 2 - 1, THEMES_DIR "/%s/", theme_name);

    if (is_dir(theme_path))
        *theme_out = theme_loadFromPath(theme_path, false);
}

void installNonDynamicElement(const char *theme_path, const char *image_name)
{
    char override_image_path[256], theme_image_path[256],
        system_image_path[256], system_image_backup[256];

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

void installTheme(Theme_s *theme, bool apply_icons)
{
    system("/mnt/SDCARD/.tmp_update/bin/mainUiBatPerc --restore");

    if (strstr(theme->path, "/.previews/") != NULL) {
        char cmd[STR_MAX * 2];
        snprintf(cmd, STR_MAX * 2 - 1,
                 SCRIPT_DIR "/themes_extract_theme.sh \"%s\"", theme->path);

        sprintf(theme->path, THEMES_DIR "/%s/", basename(theme->path));

        system(cmd);
        sync();
    }

    // change theme setting
    strcpy(settings.theme, theme->path);
    settings_save();

    FILE *fp;
    file_put_sync(fp, "/mnt/SDCARD/.tmp_update/config/active_theme", "%s",
                  theme->path);

    Theme_s with_overrides = theme_loadFromPath(theme->path, true);

    lang_removeIconLabels(with_overrides.hideLabels.icons,
                          with_overrides.hideLabels.hints);

    installNonDynamicElement(settings.theme, "bg-io-testing");
    installNonDynamicElement(settings.theme, "ic-MENU+A");
    installNonDynamicElement(settings.theme, "progress-dot");

    if (apply_icons) {
        char icon_pack_path[STR_MAX + 32];
        snprintf(icon_pack_path, STR_MAX + 32 - 1, "%sicons", theme->path);
        apply_iconPack(is_dir(icon_pack_path) ? icon_pack_path
                                              : ICON_PACK_DEFAULT);
    }
}

#endif // THEME_SWITCHER_INSTALL_THEME_H__
