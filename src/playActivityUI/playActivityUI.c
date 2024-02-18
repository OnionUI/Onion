#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "../playActivity/playActivityDB.h"
#include "components/list.h"
#include "system/battery.h"
#include "system/keymap_sw.h"
#include "system/settings.h"
#include "theme/sound.h"
#include "theme/theme.h"
#include "utils/keystate.h"
#include "utils/log.h"
#include "utils/sdl_init.h"
#include "utils/timer.h"

#define IMG_MAX_HEIGHT 80
#define IMG_MAX_WIDTH 80
#define MIN(a, b) ((a) < (b) ? (a) : (b))

static bool quit = false;

typedef struct {
    SDL_Surface *default_image;
    bool default_image_loaded;
    SDL_Surface *footer;
} UIResources;

typedef struct {
    List activity_list;
    PlayActivities *play_activities;
    KeyState keystate[320];
    bool clean_names;
    bool all_changed;
    bool header_changed;
    bool list_changed;
    bool list_content_changed;
    bool footer_changed;
    bool key_changed;
    bool battery_changed;
    int battery_percentage;
    int tmp_active_pos;
    int tmp_scroll_pos;
    UIResources *res;
} AppState;

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

//
//  Scale the image to the given width and height
//
SDL_Surface *scaleImage(SDL_Surface *img, int w, int h)
{

    double sw = (double)w / img->w;
    double sh = (double)h / img->h;
    double s = MIN(sw, sh);

    SDL_PixelFormat *ft = img->format;
    SDL_Surface *dst = SDL_CreateRGBSurface(0, (int)(s * img->w), (int)(s * img->h), ft->BitsPerPixel, ft->Rmask, ft->Gmask, ft->Bmask, ft->Amask);

    SDL_Rect src_rect = {0, 0, img->w, img->h};
    SDL_Rect dst_rect = {0, 0, dst->w, dst->h};
    SDL_SoftStretch(img, &src_rect, dst, &dst_rect);

    SDL_FreeSurface(img);
    return dst;
}

//
//  Load the image from the given path, or return the default image
//
SDL_Surface *loadRomImage(AppState *st, const char *image_path)
{
    if (!st->res->default_image_loaded) {
        st->res->default_image = IMG_Load("/mnt/SDCARD/miyoo/app/skin/thumb-default.png");
        st->res->default_image = scaleImage(st->res->default_image, IMG_MAX_WIDTH, IMG_MAX_HEIGHT);
        st->res->default_image_loaded = true;
    }

    SDL_Surface *img = is_file(image_path) ? IMG_Load(image_path) : st->res->default_image;
    return (img == st->res->default_image || !img) ? st->res->default_image : scaleImage(img, IMG_MAX_WIDTH, IMG_MAX_HEIGHT);
}

//
//  Create the activity list
//
void createActivityList(AppState *st)
{
    print_debug("Creating activity list");
    if (st->play_activities)
        free_play_activities(st->play_activities);

    START_TIMER(tm_play_activity_find_all)
    st->play_activities = play_activity_find_all(false);
    END_TIMER(tm_play_activity_find_all)

    if (st->activity_list._created)
        list_free(&st->activity_list, st->res->default_image);

    st->activity_list = list_create(st->play_activities->count, LIST_LARGE);

    START_TIMER(tm_list_addItem)
    for (int i = 0; i < st->play_activities->count; i++) {
        PlayActivity *entry = st->play_activities->play_activity[i];

        char *image_path = entry->rom->image_path;
        char rom_description[STR_MAX];
        char total[25], average[25], plays[25];
        str_serializeTime(total, entry->play_time_total);
        str_serializeTime(average, entry->play_time_average);
        snprintf(plays, 24, "%d", entry->play_count);
        snprintf(rom_description, STR_MAX, "%s ~ %s / %s plays", total, average, plays);

        ListItem item = {
            .item_type = PLAYACTIVITY,
            // .payload_ptr = entry,
            .icon_ptr = loadRomImage(st, image_path)};

        strncpy(item.description, rom_description, sizeof(item.description) - 1);

        char rom_name[STR_MAX];

        if (st->clean_names)
            strncpy(rom_name, entry->rom->name, STR_MAX - 1);
        else
            file_cleanName(rom_name, entry->rom->name);

        snprintf(item.label, STR_MAX, "%s", rom_name);
        list_addItem(&st->activity_list, item);
    }
    END_TIMER(tm_list_addItem)
    list_scrollTo(&st->activity_list, st->tmp_active_pos);
    st->activity_list.scroll_pos = st->tmp_scroll_pos;

    st->list_content_changed = false;
}

void freeResources(AppState *st)
{
    SDL_FreeSurface(st->res->default_image);
    if (st->res->footer)
        SDL_FreeSurface(st->res->footer);
    list_free(&st->activity_list, st->res->default_image);
    free_play_activities(st->play_activities);
    free(st->res);
    free(st);
}

//
// Hide the play activity
//  returns true if it wants a sound played
//
bool deletePlayActivity(AppState *st)
{
    if (st->activity_list.item_count == 0)
        return false;

    sound_change();

    // draw confirmation dialog
    START_TIMER(tm_delete_dialog)
    char message[STR_MAX];
    snprintf(message, STR_MAX, "Delete the play activity for\n%s?",
             st->play_activities->play_activity[st->activity_list.active_pos]->rom->name);
    theme_renderDialog(screen, "Remove Play Activity?", message, true);
    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);
    END_TIMER(tm_delete_dialog)

    // wait for user input
    while (!quit) {
        if (updateKeystate(st->keystate, &quit, true, NULL)) {
            if (st->keystate[SW_BTN_A] == PRESSED) {
                // delete the play activity for the selected rom
                START_TIMER(tm_delete_item)
                st->tmp_active_pos = st->activity_list.active_pos - 1 < 0 ? 0 : st->activity_list.active_pos - 1;
                play_activity_hide(st->play_activities->play_activity[st->activity_list.active_pos]->rom->id);

                // since we don't want to reload the list, fix some values manually
                st->play_activities->play_time_total -= st->play_activities->play_activity[st->activity_list.active_pos]->play_time_total;

                for (int i = st->activity_list.active_pos; i < st->play_activities->count - 1; i++) {
                    st->play_activities->play_activity[i] = st->play_activities->play_activity[i + 1];
                    st->activity_list.items[i] = st->activity_list.items[i + 1];
                }
                st->play_activities->count--;
                st->activity_list.item_count--;

                if (st->activity_list.active_pos > 0) {
                    st->activity_list.active_pos--;
                    list_scroll(&st->activity_list);
                }
                st->all_changed = true;
                END_TIMER(tm_delete_item)
                break;
            }
            if (st->keystate[SW_BTN_B] == PRESSED) {
                // cancel
                break;
            }
        }
    }
    // need to redraw everything anyway
    st->all_changed = true;
    return true;
}

bool toggleCleanNames(AppState *st)
{
    // TODO: maybe don't reload the list
    st->tmp_active_pos = st->activity_list.active_pos;
    st->tmp_scroll_pos = st->activity_list.scroll_pos;
    st->list_content_changed = st->all_changed = true;
    st->clean_names = !st->clean_names;

    return true;
}

void handleKeys(AppState *st)
{
    if (updateKeystate(st->keystate, &quit, true, NULL)) {
        if (st->keystate[SW_BTN_B] == PRESSED)
            quit = true;
        else if (st->play_activities->count > 0 && st->keystate[SW_BTN_DOWN] >= PRESSED)
            st->footer_changed = st->key_changed = list_keyDown(&st->activity_list, st->keystate[SW_BTN_DOWN] == REPEATING);
        else if (st->play_activities->count > 0 && st->keystate[SW_BTN_UP] >= PRESSED)
            st->footer_changed = st->key_changed = list_keyUp(&st->activity_list, st->keystate[SW_BTN_UP] == REPEATING);
        else if (st->play_activities->count > 0 && st->keystate[SW_BTN_X] == PRESSED && st->res->footer)
            st->key_changed = deletePlayActivity(st);
        else if (st->play_activities->count > 0 && st->keystate[SW_BTN_A] == PRESSED && !st->res->footer) // no footer means no X hint, so A is the delete option. not sure if good idea
            st->key_changed = deletePlayActivity(st);
        else if (st->play_activities->count > 0 && st->keystate[SW_BTN_Y] == PRESSED)
            st->key_changed = toggleCleanNames(st);

        st->list_changed = st->list_changed || st->key_changed;
    }

    if (st->key_changed)
        sound_change();

    st->key_changed = false;
}

//
//  In case there are no games played yet, looks better than an empty list
//
void renderNoGamesInfo(AppState *st)
{
    // clear the list area, important in case all entries were deleted
    SDL_FillRect(screen, &(SDL_Rect){0, 60, RENDER_WIDTH, RENDER_HEIGHT - st->res->footer->h}, 0);

    // no games played yet
    SDL_Surface *text_line = TTF_RenderUTF8_Blended(resource_getFont(HINT), "No games played yet", theme()->hint.color);
    int text_x = (screen->w - text_line->w) / 2;
    int text_y = (screen->h - text_line->h) / 2 - 20;
    SDL_BlitSurface(text_line, NULL, screen, &(SDL_Rect){text_x, text_y, 0, 0});
    SDL_FreeSurface(text_line);

    // time to play
    text_line = TTF_RenderUTF8_Blended(resource_getFont(HINT), "Time to play!", theme()->hint.color);
    text_x = (screen->w - text_line->w) / 2;
    text_y = (screen->h - text_line->h) / 2 + 20;
    SDL_BlitSurface(text_line, NULL, screen, &(SDL_Rect){text_x, text_y, 0, 0});
    SDL_FreeSurface(text_line);
}

void renderLoadingText()
{
    TTF_Font *font40 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 40);
    SDL_Surface *loadingText = TTF_RenderUTF8_Blended(font40, "Loading...", (SDL_Color){255, 255, 255});
    int text_x = (screen->w - loadingText->w) / 2;
    int text_y = (screen->h - loadingText->h) / 2;
    SDL_BlitSurface(loadingText, NULL, screen, &(SDL_Rect){text_x, text_y, 0, 0});
    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_FreeSurface(loadingText);
    TTF_CloseFont(font40);
    SDL_Flip(screen);
}

//
// Render current/total number of items on the footer
//
void renderCustomFooterInfo(AppState *st)
{
    char total_items_str[STR_MAX];
    snprintf(total_items_str, STR_MAX, "%d/%d", st->activity_list.active_pos + 1, st->activity_list.item_count);
    SDL_Surface *total_items = TTF_RenderUTF8_Blended(resource_getFont(HINT), total_items_str, theme()->hint.color);
    SDL_Rect total_items_rect = {RENDER_WIDTH - total_items->w - 20, RENDER_HEIGHT - st->res->footer->h / 2 - total_items->h / 2};
    SDL_BlitSurface(total_items, NULL, screen, &total_items_rect);
    SDL_FreeSurface(total_items);
}

AppState *init()
{
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    log_setName("playActivityUI");
    print_debug("Debug logging enabled");
    getDeviceModel();
    SDL_InitDefault(true);

    renderLoadingText();

    settings_load();
    theme_load();

    AppState *st = calloc(1, sizeof(AppState));
    if (!st) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    st->res = calloc(1, sizeof(UIResources));
    if (!st->res) {
        perror("malloc");
        free(st);
        exit(EXIT_FAILURE);
    }
    st->all_changed = st->list_content_changed = true;
    // load custom footer if available
    char footer_path[STR_MAX];
    if (theme_getImagePath(theme()->path, "extra/at-bottom-bar", footer_path)) {
        st->res->footer = IMG_Load(footer_path);
        if (!st->res->footer)
            printf("Error loading footer image: %s\n", IMG_GetError());
    }
    else {
        st->res->footer = NULL;
    }

    return st;
}

int main()
{
    static bool first = true;

    START_TIMER(tm_loading);

    START_TIMER(tm_init);
    AppState *st = init();
    END_TIMER(tm_init);

    while (!quit) {

        handleKeys(st);

        if (battery_hasChanged(0, &st->battery_percentage))
            st->battery_changed = true;

        // happens on first run and when toggling clean names
        if (st->list_content_changed)
            createActivityList(st);

        if (st->header_changed || st->battery_changed || st->all_changed) {
            START_TIMER(tm_renderHeader);
            char total_str[25] = "Total: ";
            char total[25];
            str_serializeTime(total, st->play_activities->play_time_total);
            strcat(total_str, total);
            theme_renderHeader(screen, total_str, false);
            theme_renderHeaderBattery(screen, st->battery_percentage);
            END_TIMER(tm_renderHeader);
        }

        if (st->list_changed || st->all_changed) {
            START_TIMER(tm_renderList);
            if (st->play_activities->count > 0)
                theme_renderList(screen, &st->activity_list);
            else
                renderNoGamesInfo(st);

            END_TIMER(tm_renderList);
        }

        if (st->footer_changed || st->all_changed) {
            START_TIMER(tm_renderFooter);
            if (st->res->footer) {
                SDL_Rect footer_rect = {0, RENDER_HEIGHT - st->res->footer->h};
                SDL_BlitSurface(st->res->footer, NULL, screen,
                                &footer_rect);
                if (st->play_activities->count > 0)
                    renderCustomFooterInfo(st);
            }
            else {
                theme_renderFooter(screen);
                theme_renderStandardHint(screen, "DELETE", "EXIT");
                theme_renderFooterStatus(screen, st->activity_list.active_pos + 1, st->activity_list.item_count);
            }

            END_TIMER(tm_renderFooter);
        }

        // if anything changed, update the screen
        if (st->list_changed || st->header_changed || st->footer_changed || st->all_changed || st->battery_changed) {
            print_debug("Blitting");
            SDL_BlitSurface(screen, NULL, video, NULL);
            SDL_Flip(video);
            st->list_changed = st->header_changed = st->footer_changed = st->all_changed = st->battery_changed = false;
        }
        if (first) {
            first = false;
            END_TIMER(tm_loading);
        }
    }

    SDL_FillRect(video, NULL, 0);
    SDL_Flip(video);

    freeResources(st);
    return EXIT_SUCCESS;
}