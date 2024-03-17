#ifndef TWEAKS_ICONS_H__
#define TWEAKS_ICONS_H__

#include <SDL/SDL_image.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "components/list.h"
#include "system/keymap_sw.h"
#include "theme/render/dialog.h"
#include "theme/sound.h"
#include "utils/apply_icons.h"
#include "utils/json.h"
#include "utils/keystate.h"

#include "./appstate.h"

typedef struct IconInfo {
    char name[STR_MAX];
    char path[STR_MAX];
    char config_path[STR_MAX];
} IconInfo_t;

static IconInfo_t icon_infos[500];
static int icon_infos_len = 0;

int _add_icon_alts(const char *pack_dir, const char *pack_name,
                   const char *icon_prefix, List *list, void (*action)(void *))
{
    DIR *dp;
    struct dirent *ep;
    char icon_name[STR_MAX];
    char alt_name[STR_MAX];
    char preview_path[STR_MAX * 2 + 1];
    int count = 0;

    if ((dp = opendir(pack_dir)) != NULL) {
        while ((ep = readdir(dp))) {
            if (ep->d_type != DT_REG)
                continue;
            if (strncmp(icon_prefix, ep->d_name, strlen(icon_prefix)) != 0 ||
                ep->d_name[strlen(icon_prefix)] != '-')
                continue;

            snprintf(preview_path, STR_MAX * 2, "%s/%s", pack_dir, ep->d_name);

            strncpy(icon_name, ep->d_name, STR_MAX - 1);
            snprintf(alt_name, STR_MAX - 1, "%s - %s", pack_name,
                     file_removeExtension(str_split(icon_name, "-")));

            ListItem item = {.action = action};
            strncpy(item.label, alt_name, STR_MAX - 1);
            strncpy(item.payload, pack_dir, STR_MAX - 1);

            if (is_file(preview_path))
                strncpy(item.preview_path, preview_path, STR_MAX - 1);

            list_addItem(list, item);
            count++;
        }
        closedir(dp);
    }

    return count;
}

int _add_icon_packs(const char *path, List *list, void (*action)(void *),
                    bool is_theme, const char *required_icon)
{
    DIR *dp;
    struct dirent *ep;
    char icon_pack_name[STR_MAX];
    char icon_pack_path[STR_MAX * 2];
    char preview_path[STR_MAX * 2 + 32];
    int count = 0;

    if ((dp = opendir(path)) != NULL) {
        while ((ep = readdir(dp))) {
            if (ep->d_type != DT_DIR)
                continue;
            if (ep->d_name[0] == '.')
                continue;
            if (strcmp("sel", ep->d_name) == 0)
                continue;
            if (strcmp("app", ep->d_name) == 0)
                continue;
            if (strcmp("rapp", ep->d_name) == 0)
                continue;
            if (strcmp("icons", ep->d_name) == 0)
                continue;

            snprintf(icon_pack_path, STR_MAX * 2 - 1,
                     is_theme ? "%s/%s/icons" : "%s/%s", path, ep->d_name);

            if (required_icon != NULL) {
                snprintf(preview_path, STR_MAX * 2 + 31, "%s/%s.png",
                         icon_pack_path, required_icon);

                if (!is_file(preview_path))
                    continue;
            }
            else {
                snprintf(preview_path, STR_MAX * 2 + 31, "%s/preview.png",
                         icon_pack_path);

                if (!is_file(preview_path))
                    snprintf(preview_path, STR_MAX * 2 + 31, "%s/gba.png",
                             icon_pack_path);
            }

            if (is_dir(icon_pack_path)) {
                strncpy(icon_pack_name, ep->d_name, STR_MAX - 1);
                str_split(icon_pack_name, " by ");

                ListItem item = {.action = action};
                strncpy(item.label, icon_pack_name, STR_MAX - 1);
                strncpy(item.payload, icon_pack_path, STR_MAX - 1);

                if (is_file(preview_path))
                    strncpy(item.preview_path, preview_path, STR_MAX - 1);

                list_addItem(list, item);
                count++;

                if (required_icon != NULL)
                    count += _add_icon_alts(icon_pack_path, icon_pack_name,
                                            required_icon, list, action);
            }
        }
        closedir(dp);
    }

    return count;
}

void _action_apply_icon_pack(void *_item)
{
    ListItem *item = (ListItem *)_item;

    bool apply = false;
    bool confirm_quit = false;
    SDLKey changed_key = SDLK_UNKNOWN;

    keys_enabled = false;

    SDL_Surface *background_cache =
        SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);
    SDL_BlitSurface(screen, NULL, background_cache, NULL);

    theme_renderDialog(screen, item->label,
                       "Do you want to apply\nthis icon pack?", true);
    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);

    while (!confirm_quit) {
        if (updateKeystate(keystate, &confirm_quit, true, &changed_key)) {
            if (changed_key == SW_BTN_B && keystate[SW_BTN_B] == PRESSED)
                confirm_quit = true;
            else if (changed_key == SW_BTN_A && keystate[SW_BTN_A] == PRESSED) {
                apply = true;
                confirm_quit = true;
            }
        }
    }

    if (changed_key != SDLK_UNKNOWN) {
        sound_change();
    }

    if (apply) {
        char message_done[STR_MAX];
        int applied = apply_iconPack(item->payload, false);

        sprintf(message_done, "Applied %d icons", applied);

        list_free(&_menu_console_icons, NULL);
        list_free(&_menu_app_icons, NULL);
        list_free(&_menu_temp, NULL);

        SDL_BlitSurface(background_cache, NULL, screen, NULL);
        theme_renderDialog(screen, item->label, message_done, false);
        SDL_BlitSurface(screen, NULL, video, NULL);
        SDL_Flip(video);
        msleep(1000);

        reset_menus = true;
    }

    SDL_FreeSurface(background_cache);
    keys_enabled = true;
    all_changed = true;
}

void menu_icon_packs(void *_)
{
    const char *active_icon_pack = file_read(ACTIVE_ICON_PACK);

    if (!_menu_icon_packs._created) {
        _menu_icon_packs = list_create(200, LIST_SMALL);
        strcpy(_menu_icon_packs.title, "Icon packs");

        _add_icon_packs("/mnt/SDCARD/Icons", &_menu_icon_packs,
                        _action_apply_icon_pack, false, NULL);
        _add_icon_packs("/mnt/SDCARD/Themes", &_menu_icon_packs,
                        _action_apply_icon_pack, true, NULL);

        list_sortByLabel(&_menu_icon_packs);

        char selected_path[STR_MAX];
        realpath(is_dir(active_icon_pack) ? active_icon_pack
                                          : "/mnt/SDCARD/Icons/Default",
                 selected_path);

        for (int i = 0; i < _menu_icon_packs.item_count; i++) {
            ListItem *current_item = &_menu_icon_packs.items[i];

            char current_path[STR_MAX];
            realpath(current_item->payload, current_path);

            if (strcmp(selected_path, current_path) == 0) {
                list_scrollTo(&_menu_icon_packs, i);
                break;
            }
        }
    }
    menu_stack[++menu_level] = &_menu_icon_packs;
    header_changed = true;
}

bool _add_config_icon(const char *path, const char *name,
                      const char *config_path, List *list,
                      void (*action)(void *))
{
    char label[STR_MAX];
    char icon_name[56];
    char icon_path[STR_MAX];
    char preview_path[STR_MAX * 2 + 32];
    IconMode_e mode = icons_getIconMode(config_path);

    if (!is_file(config_path))
        return false;

    cJSON *config = json_load(config_path);

    if (!json_getString(config, "icon", icon_path)) {
        cJSON_free(config);
        return false;
    }

    if (!json_getString(config, "label", label))
        strncpy(label, name, STR_MAX - 1);

    cJSON_free(config);

    ListItem item = {.action = action};

    if (icon_path[0] != '/')
        snprintf(preview_path, STR_MAX * 2 + 32 - 1, "%s/%s/%s", path, name,
                 icon_path);
    else
        strncpy(preview_path, icon_path, STR_MAX * 2 - 1);

    char abs_path[STR_MAX - 56];
    realpath(preview_path, abs_path);

    strncpy(icon_name, file_removeExtension(basename(icon_path)), 55);
    str_split(icon_name, "-");

    char short_label[56];
    str_trim(short_label, 55, label, false);
    short_label[56] = 0;

    if (mode != ICON_MODE_APP)
        snprintf(item.label, STR_MAX - 1, "%s (%s)", short_label, icon_name);
    else {
        strncpy(item.label, short_label, STR_MAX - 1);
        strncpy(item.description, icon_name, STR_MAX - 1);
    }

    IconInfo_t *info = &icon_infos[icon_infos_len++];
    strcpy(info->name, icon_name);
    strncpy(info->path, preview_path, STR_MAX - 1);
    strncpy(info->config_path, config_path, STR_MAX - 1);
    item.payload_ptr = (void *)info;

    if (mode != ICON_MODE_APP)
        strncpy(item.preview_path, preview_path, STR_MAX - 1);
    else
        item.icon_ptr = (void *)IMG_Load(preview_path);

    list_addItem(list, item);

    return true;
}

int _add_config_icons(const char *path, List *list, void (*action)(void *))
{
    DIR *dp;
    struct dirent *ep;
    char config_path[STR_MAX * 2];
    int count = 0;

    if ((dp = opendir(path)) != NULL) {
        while ((ep = readdir(dp))) {
            if (ep->d_type != DT_DIR)
                continue;
            if (ep->d_name[0] == '.')
                continue;
            if (strcmp("romscripts", ep->d_name) == 0)
                continue;

            snprintf(config_path, STR_MAX * 2 - 1, "%s/%s/config.json", path,
                     ep->d_name);

            if (strcmp(SEARCH_CONFIG, config_path) == 0)
                continue;
            if (strcmp(GUEST_CONFIG, config_path) == 0)
                continue;

            if (!is_file(config_path))
                continue;

            if (_add_config_icon(path, ep->d_name, config_path, list, action))
                count++;
        }
        closedir(dp);
    }

    return count;
}

static ListItem *temp_action_item = NULL;

void _menu_temp_action(void *_item)
{
    ListItem *item = (ListItem *)_item;
    IconInfo_t *info = (IconInfo_t *)temp_action_item->payload_ptr;
    IconMode_e mode = icons_getIconMode(info->config_path);

    if (strcmp(info->path, item->preview_path) != 0) {
        keys_enabled = false;

        apply_singleIconByFullPath(info->config_path, item->preview_path);
        strcpy(info->path, item->preview_path);

        if (mode != ICON_MODE_APP) {
            strcpy(temp_action_item->preview_path, item->preview_path);
            if (temp_action_item->preview_ptr != NULL) {
                SDL_FreeSurface((SDL_Surface *)temp_action_item->preview_ptr);
                temp_action_item->preview_ptr = NULL;
            }
        }
        else {
            if (temp_action_item->icon_ptr != NULL)
                SDL_FreeSurface((SDL_Surface *)temp_action_item->icon_ptr);
            temp_action_item->icon_ptr = (void *)IMG_Load(item->preview_path);
        }

        theme_renderDialog(screen, item->label, "Change saved", false);
        SDL_BlitSurface(screen, NULL, video, NULL);
        SDL_Flip(video);
        msleep(500);

        reset_menus = true;
    }

    menu_stack[menu_level] = NULL;
    menu_level--;
    keys_enabled = true;
    all_changed = true;
}

void _menu_change_icon(ListItem *item, IconMode_e mode)
{
    if (_menu_temp._created)
        list_free(&_menu_temp, NULL);

    _menu_temp = list_create(200, LIST_SMALL);
    strncpy(_menu_temp.title, item->label, STR_MAX - 1);

    IconInfo_t *info = (IconInfo_t *)item->payload_ptr;

    char required_icon[STR_MAX];
    snprintf(required_icon, STR_MAX - 1, icons_getIconNameFormat(mode),
             info->name);

    _add_icon_packs("/mnt/SDCARD/Icons", &_menu_temp, _menu_temp_action, false,
                    required_icon);
    _add_icon_packs("/mnt/SDCARD/Themes", &_menu_temp, _menu_temp_action, true,
                    required_icon);

    list_sortByLabel(&_menu_temp);

    char selected_path[STR_MAX];
    realpath(info->path, selected_path);

    for (int i = 0; i < _menu_temp.item_count; i++) {
        ListItem *current_item = &_menu_temp.items[i];

        char current_path[STR_MAX];
        realpath(current_item->preview_path, current_path);

        if (strcmp(selected_path, current_path) == 0) {
            list_scrollTo(&_menu_temp, i);
            break;
        }
    }

    temp_action_item = item;

    menu_stack[++menu_level] = &_menu_temp;
    header_changed = true;
}

void menu_change_console_icon(void *item)
{
    _menu_change_icon((ListItem *)item, ICON_MODE_EMU);
}

void menu_console_icons(void *_)
{
    if (!_menu_console_icons._created) {
        _menu_console_icons = list_create(200, LIST_SMALL);
        strcpy(_menu_console_icons.title, "Console icons");

        _add_config_icons(CONFIG_EMU_PATH, &_menu_console_icons,
                          menu_change_console_icon);
        _add_config_icon(CONFIG_EMU_PATH, "SEARCH", SEARCH_CONFIG_SRC,
                         &_menu_console_icons, menu_change_console_icon);

        list_sortByLabel(&_menu_console_icons);
    }
    menu_stack[++menu_level] = &_menu_console_icons;
    header_changed = true;
}

void menu_change_app_icon(void *item)
{
    _menu_change_icon((ListItem *)item, ICON_MODE_APP);
}

void menu_app_icons(void *_)
{
    if (!_menu_app_icons._created) {
        _menu_app_icons = list_create(200, LIST_LARGE);
        strcpy(_menu_app_icons.title, "App icons");

        _add_config_icons(CONFIG_APP_PATH, &_menu_app_icons,
                          menu_change_app_icon);
        _add_config_icon(CONFIG_APP_PATH, "Guest_Mode", GUEST_OFF_CONFIG,
                         &_menu_app_icons, menu_change_app_icon);
        _add_config_icon(CONFIG_APP_PATH, "Guest_Mode", GUEST_ON_CONFIG,
                         &_menu_app_icons, menu_change_app_icon);

        list_sortByLabel(&_menu_app_icons);
    }
    menu_stack[++menu_level] = &_menu_app_icons;
    header_changed = true;
}

void menu_change_expert_icon(void *item)
{
    _menu_change_icon((ListItem *)item, ICON_MODE_RAPP);
}

void menu_expert_icons(void *_)
{
    if (!_menu_expert_icons._created) {
        _menu_expert_icons = list_create(200, LIST_SMALL);
        strcpy(_menu_expert_icons.title, "Expert icons");

        _add_config_icons(CONFIG_RAPP_PATH, &_menu_expert_icons,
                          menu_change_expert_icon);

        list_sortByLabel(&_menu_expert_icons);
    }
    menu_stack[++menu_level] = &_menu_expert_icons;
    header_changed = true;
}

void menu_icons(void *_)
{
    if (!_menu_icons._created) {
        _menu_icons = list_create(5, LIST_SMALL);
        strcpy(_menu_icons.title, "Icons");
        list_addItem(&_menu_icons, (ListItem){.label = "Apply icon pack...",
                                              .action = menu_icon_packs});
        list_addItem(&_menu_icons, (ListItem){.label = "Edit console icon...",
                                              .action = menu_console_icons});
        list_addItem(&_menu_icons, (ListItem){.label = "Edit app icon...",
                                              .action = menu_app_icons});
        list_addItem(&_menu_icons, (ListItem){.label = "Edit expert icon...",
                                              .action = menu_expert_icons});
    }
    menu_stack[++menu_level] = &_menu_icons;
    header_changed = true;
}

#endif // TWEAKS_ICONS_H__
