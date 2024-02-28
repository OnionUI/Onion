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
#include <../playActivity/playActivityDB.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <signal.h>
#include <sqlite3/sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PLAY_ACTIVITY_DB_PATH "/mnt/SDCARD/Saves/CurrentProfile/play_activity/play_activity_db.sqlite"
#define HIDE_SPLASH_FLAG "/mnt/SDCARD/App/EmuSort/hide_splash"
#define EMU_DIR "/mnt/SDCARD/Emu"
#define MAX_EMU_COUNT 200
#define MAX_LABEL_LEN 100
#define IMG_MAX_WIDTH 80
#define IMG_MAX_HEIGHT 80
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

static bool quit = false;

typedef struct {
    char label[MAX_LABEL_LEN];
    char padded_label[MAX_LABEL_LEN];
    char path[STR_MAX];
    char rompath[STR_MAX];
    int playtime;
    SDL_Surface *icon;
    SDL_Surface *icon_sel;
} EmuInfo;

typedef struct {
    List emu_list;
    EmuInfo emus[MAX_EMU_COUNT];
    KeyState keystate[320];
    int ticks;
    int tmp_active_pos;
    int tmp_scroll_pos;
    int emu_count;
    int battery_percentage;
    bool sort_direction;
    bool all_changed;
    bool header_changed;
    bool list_changed;
    bool list_content_changed;
    bool key_changed;
    bool battery_changed;
    char *theme_path;
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

void blitFlip()
{
    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);
}

//
// Frees list without freeing icon_ptr
//
void listFree(List *list)
{
    if (!list->_created)
        return;

    free(list->items);
    list->_created = false;
}

void freeResources(AppState *st)
{

    for (int i = 0; i < st->emu_count; i++) {
        if (st->emus[i].icon) {
            if (st->emus[i].icon == st->emus[i].icon_sel)
                st->emus[i].icon_sel = NULL; // avoid double free if icon and icon_sel point to the same surface
            SDL_FreeSurface(st->emus[i].icon);
        }
        if (st->emus[i].icon_sel)
            SDL_FreeSurface(st->emus[i].icon_sel);
    }
    if (st->emu_list._created)
        listFree(&st->emu_list);
    free(st);
}

void showHelp(AppState *st)
{
    theme_renderDialog(screen, "EmuSort", "[L2/R2] Move emulators\n[X] Sort by A-Z/Z-A\n[Y] Sort by playtime\n \n[A] Continue", false);
    blitFlip();
    while (!quit)
        if (updateKeystate(st->keystate, &quit, true, NULL) && st->keystate[SW_BTN_A] == PRESSED)
            break;

    st->header_changed = st->list_changed = true;
}

void showSplashScreen(AppState *st)
{
    theme_renderDialog(screen, "EmuSort", "[L2/R2] Move emulators\n[X] Sort by A-Z/Z-A\n[Y] Sort by playtime\n[START] Show help\n[SELECT] Don't show this hint again\n[Any key] Continue", false);
    blitFlip();
    while (!quit) {
        if (updateKeystate(st->keystate, &quit, true, NULL)) {
            if (st->keystate[SW_BTN_SELECT] == PRESSED) {
                FILE *file = fopen(HIDE_SPLASH_FLAG, "w");
                if (file)
                    fclose(file);
            }
            break;
        }
    }
}

AppState *init()
{
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    log_setName("emuSort");
    print_debug("Debug logging enabled");
    getDeviceModel();
    SDL_InitDefault(true);

    settings_load();
    theme_load();
    AppState *st = calloc(1, sizeof(AppState));
    if (!st) {
        fprintf(stderr, "Error allocating memory for AppState\n");
        exit(EXIT_FAILURE);
    }

    st->all_changed = true;
    st->theme_path = theme()->path;
    st->battery_percentage = battery_getPercentage();
    return st;
}

//
// comparators for qsort
//
static int compareEmuLabelPadded(const void *a, const void *b)
{
    return strcmp(((EmuInfo *)a)->padded_label, ((EmuInfo *)b)->padded_label);
}
static int compareEmuLabel(const void *a, const void *b)
{
    return strcmp(((EmuInfo *)a)->label, ((EmuInfo *)b)->label);
}
static int compareEmuPlaytime(const void *a, const void *b)
{
    return ((EmuInfo *)b)->playtime - ((EmuInfo *)a)->playtime;
}

//
// Swap the active item with the one above or below it
//
bool swapItems(AppState *st, int direction)
{
    // upper and lower bounds
    if (direction < 0 && st->emu_list.active_pos == 0)
        return false;
    else if (direction > 0 && st->emu_list.active_pos == st->emu_count - 1)
        return false;

    // save the current positions to restore them after
    st->tmp_scroll_pos = st->emu_list.scroll_pos;
    st->tmp_active_pos = st->emu_list.active_pos + direction;

    // swap the items
    EmuInfo tmp = st->emus[st->emu_list.active_pos];
    st->emus[st->emu_list.active_pos] = st->emus[st->emu_list.active_pos + direction];
    st->emus[st->emu_list.active_pos + direction] = tmp;

    // mark list as changed
    st->list_content_changed = true;
    return true;
}

//
// Pad string with pad_amount spaces, returning a new string
//
char *padString(const char *input, size_t pad_amount)
{
    size_t input_length = strlen(input);
    size_t padded_length = input_length + 2 * pad_amount;

    char *padded_string = (char *)malloc(padded_length + 1);
    if (padded_string == NULL) {
        fprintf(stderr, "Error allocating memory for padded string, input: %s\n", input);
        exit(EXIT_FAILURE);
    }

    // leading spaces
    for (size_t i = 0; i < pad_amount; i++)
        padded_string[i] = ' ';

    // copy of input string
    strcpy(padded_string + pad_amount, input);

    // trailing spaces
    for (size_t i = input_length + pad_amount; i < padded_length; i++)
        padded_string[i] = ' ';

    padded_string[padded_length] = '\0';

    return padded_string;
}

//
// Save the emulators config files with the padded labels
//
void saveItems(AppState *st)
{
    theme_renderDialog(screen, "Saving...", "Please wait", false);
    blitFlip();

    for (int i = 0; i < st->emu_count; i++) {
        // read the existing JSON file
        FILE *file = fopen(st->emus[i].path, "r");
        if (!file) {
            fprintf(stderr, "Error opening file for reading: %s\n", st->emus[i].path);
            continue;
        }

        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        char *buffer = (char *)malloc(file_size + 1);
        if (!buffer) {
            fclose(file);
            fprintf(stderr, "Error allocating memory for file: %s\n", st->emus[i].path);
            continue;
        }

        fread(buffer, 1, file_size, file);
        fclose(file);

        buffer[file_size] = '\0';

        cJSON *root = cJSON_Parse(buffer);
        free(buffer);

        if (!root) {
            fprintf(stderr, "Error parsing JSON in file: %s\n", st->emus[i].path);
            continue;
        }

        int num_spaces = st->emu_count - i - 1;

        // add spaces
        char *spaced_label = padString(st->emu_list.items[i].label, num_spaces);
        // printf("Updating label from \"%s\" to \"%s\"\n", st->emu_list.items[i].label, spaced_label);

        // write padded label back to the cJSON object
        cJSON_ReplaceItemInObject(root, "label", cJSON_CreateString(spaced_label));
        free(spaced_label);

        // convert the cJSON object to a formatted JSON string
        char *json_str = cJSON_Print(root);
        cJSON_Delete(root);

        // write the JSON string back to the file
        file = fopen(st->emus[i].path, "w");
        if (!file) {
            fprintf(stderr, "Error opening file for writing: %s\n", st->emus[i].path);
            free(json_str);
            continue;
        }

        fprintf(file, "%s\n", json_str);
        fsync(fileno(file));
        fclose(file);
        free(json_str);
    }

    // feedback for user then quit
    theme_renderDialog(screen, "Done!", "Items have been sorted", false);
    blitFlip();
    sleep(1);
    quit = true;
}

//
// Sort the emulators alphabetically, alternating direction
//
bool sortAlphabetically(AppState *st)
{
    START_TIMER(tm_sort);
    qsort(st->emus, st->emu_count, sizeof(st->emus[0]), compareEmuLabel);
    if (st->sort_direction) { // Z-A: reverse
        for (int i = 0; i < st->emu_count / 2; i++) {
            EmuInfo tmp = st->emus[i];
            st->emus[i] = st->emus[st->emu_count - i - 1];
            st->emus[st->emu_count - i - 1] = tmp;
        }
    }

    st->tmp_active_pos = st->tmp_scroll_pos = 0;
    st->sort_direction = !st->sort_direction;
    st->all_changed = true;
    END_TIMER(tm_sort);
    return true;
}

//
// Sort the emulators by playtime
//
bool sortByPlaytime(AppState *st)
{
    START_TIMER(tm_sort);
    qsort(st->emus, st->emu_count, sizeof(st->emus[0]), compareEmuPlaytime);
    st->tmp_active_pos = st->tmp_scroll_pos = 0;
    st->all_changed = true;
    END_TIMER(tm_sort);
    return true;
}

void handleKeys(AppState *st)
{
    if (updateKeystate(st->keystate, &quit, true, NULL)) {

        if (st->keystate[SW_BTN_B] == PRESSED) // quit
            quit = true;
        else if (st->keystate[SW_BTN_A] == PRESSED) // save & quit
            saveItems(st);
        else if (st->keystate[SW_BTN_DOWN] >= PRESSED) // scroll down
            st->key_changed = list_keyDown(&st->emu_list, st->keystate[SW_BTN_DOWN] == REPEATING);
        else if (st->keystate[SW_BTN_UP] >= PRESSED) // scroll up
            st->key_changed = list_keyUp(&st->emu_list, st->keystate[SW_BTN_UP] == REPEATING);
        else if (st->keystate[SW_BTN_L2] >= PRESSED) // move selected item up
            st->key_changed = swapItems(st, -1);
        else if (st->keystate[SW_BTN_R2] >= PRESSED) // move selected item down
            st->key_changed = swapItems(st, 1);
        else if (st->keystate[SW_BTN_X] == PRESSED) // sort alphabetically
            st->key_changed = sortAlphabetically(st);
        else if (st->keystate[SW_BTN_Y] == PRESSED) // sort by playtime
            st->key_changed = sortByPlaytime(st);
        else if (st->keystate[SW_BTN_START] == PRESSED) // show help
            showHelp(st);

        st->list_changed = st->list_changed || st->key_changed;
    }

    if (st->key_changed)
        sound_change();

    st->key_changed = false;
}

void trimWhitespace(char *str)
{
    if (str == NULL)
        return;
    size_t start = 0;
    size_t end = strlen(str);

    while (start < end && isspace(str[start]))
        start++;
    while (end > start && isspace(str[end - 1]))
        end--;

    memmove(str, str + start, end - start);
    str[end - start] = '\0';
}

//
// Builds the menu list from emu_list
// Free the list if it was already created
//
void buildList(AppState *st)
{
    START_TIMER(tm_buildList);
    if (st->emu_list._created)
        listFree(&st->emu_list);

    st->emu_list = list_create(st->emu_count, LIST_LARGE);

    for (int i = 0; i < st->emu_count; i++) {
        ListItem item = {.icon_ptr = st->emus[i].icon};
        snprintf(item.label, STR_MAX, "%s", st->emus[i].label);
        list_addItem(&st->emu_list, item);
    }
    END_TIMER(tm_buildList);
}

const char *getFilename(const char *path)
{
    const char *filename = strrchr(path, '/');
    return (filename != NULL) ? filename + 1 : path;
}

//
// Load the emu icon and scale it fit the list
//
void loadEmuIcons(AppState *st, const char *image_path)
{
    char themed_icon_path[STR_MAX];
    SDL_Surface *img = NULL;
    // try to load the themed icon, if it doesn't exist, use the default one
    if (!(image_path[0] == '/')) {
        // not an absolute path, construct the path from the config.json base dir

        char basedir[STR_MAX];
        strcpy(basedir, st->emus[st->emu_count].path);
        char *dir = strrchr(basedir, '/');
        if (dir != NULL) {
            *(dir + 1) = '\0';
        }
        snprintf(themed_icon_path, sizeof(themed_icon_path), "%s%s", basedir, image_path);
        img = IMG_Load(themed_icon_path);
    }
    else {
        // absolute path, use themed icon if available
        snprintf(themed_icon_path, sizeof(themed_icon_path), "%sicons/%s", st->theme_path, getFilename(image_path));
        img = IMG_Load(is_file(themed_icon_path) ? themed_icon_path : image_path);
    }

    if (img == NULL) {
        // we have no icon, that's fine as well
        fprintf(stderr, "Error loading icon: %s\n", image_path);
        fprintf(stderr, "SDL Error: %s\n", IMG_GetError());
        return;
    }

    // scale the icon to fit the list
    double sw = (double)IMG_MAX_WIDTH / img->w;
    double sh = (double)IMG_MAX_HEIGHT / img->h;
    double scale = MIN(sw, sh);
    SDL_Surface *scaled_img = rotozoomSurface(img, 0.0, scale, true);
    if (scaled_img == NULL) {
        fprintf(stderr, "rotozoomSurface failed: %s\n", SDL_GetError());
    }
    else {
        SDL_FreeSurface(img);
        img = scaled_img;
    }
    st->emus[st->emu_count].icon = img;

    // load the "selected" icon if available
    snprintf(themed_icon_path, sizeof(themed_icon_path), "%sicons/sel/%s", st->theme_path, getFilename(image_path));
    if (is_file(themed_icon_path)) {
        img = IMG_Load(is_file(themed_icon_path) ? themed_icon_path : image_path);
        sw = (double)IMG_MAX_WIDTH / img->w;
        sh = (double)IMG_MAX_HEIGHT / img->h;
        scale = MIN(sw, sh);
        scaled_img = rotozoomSurface(img, 0.0, scale, true);
        if (scaled_img == NULL) {
            fprintf(stderr, "rotozoomSurface failed: %s\n", SDL_GetError());
        }
        else {
            SDL_FreeSurface(img);
            img = scaled_img;
        }
        st->emus[st->emu_count].icon_sel = img;
    }
    else {
        // if no "selected" icon is available, use the normal one
        st->emus[st->emu_count].icon_sel = st->emus[st->emu_count].icon;
    }
}

void processConfig(const char *configPath, AppState *st)
{
    // load and parse config file
    FILE *file = fopen(configPath, "r");
    if (!file) {
        fprintf(stderr, "Error opening file: %s\n", configPath);
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(file_size + 1);
    if (!buffer) {
        fclose(file);
        fprintf(stderr, "Error allocating memory for file: %s\n", configPath);
        return;
    }

    fread(buffer, 1, file_size, file);
    fclose(file);

    buffer[file_size] = '\0';

    cJSON *root = cJSON_Parse(buffer);
    free(buffer);

    if (!root) {
        fprintf(stderr, "Error parsing JSON in file: %s\n", configPath);
        return;
    }

    // get label
    cJSON *labelObj = cJSON_GetObjectItem(root, "label");
    if (labelObj && cJSON_IsString(labelObj) && strlen(labelObj->valuestring) > 0) {

        // store label with spaces
        strncpy(st->emus[st->emu_count].padded_label, labelObj->valuestring, MAX_LABEL_LEN - 1);
        st->emus[st->emu_count].padded_label[MAX_LABEL_LEN - 1] = '\0';

        // store label without spaces
        strncpy(st->emus[st->emu_count].label, labelObj->valuestring, MAX_LABEL_LEN - 1);
        st->emus[st->emu_count].label[MAX_LABEL_LEN - 1] = '\0';
        trimWhitespace(st->emus[st->emu_count].label);

        // get icon
        cJSON *iconObj = cJSON_GetObjectItem(root, "icon");
        if (iconObj && cJSON_IsString(iconObj)) {

            loadEmuIcons(st, iconObj->valuestring);
            if (!st->emus[st->emu_count].icon) {
                fprintf(stderr, "Error loading icon: %s\n", iconObj->valuestring);
                fprintf(stderr, "SDL Error: %s\n", IMG_GetError());
            }
        }
        // get rompath
        cJSON *rompathObj = cJSON_GetObjectItem(root, "rompath");
        if (rompathObj && cJSON_IsString(rompathObj)) {
            // rompaths look like this:  "../../Roms/PS" we only want "PS"
            char *rompath = strrchr(rompathObj->valuestring, '/');
            if (rompath != NULL) {
                strncpy(st->emus[st->emu_count].rompath, rompath + 1, STR_MAX - 1);
                st->emus[st->emu_count].rompath[STR_MAX - 1] = '\0';
            }
        }
        st->emu_count++;
    }
    else {
        fprintf(stderr, ":: Error reading config file: %s\n", configPath);
    }

    cJSON_Delete(root);
}

void processDirectories(const char *base_dir, AppState *st)
{
    DIR *dir = opendir(base_dir);
    if (!dir) {
        fprintf(stderr, "Error opening directory: %s\n", base_dir);
        return;
    }

    struct dirent *entry;

    // iterate through the directory, skipping . and ..
    while ((entry = readdir(dir)) != NULL && st->emu_count < MAX_EMU_COUNT) {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char configPath[STR_MAX * 2];

            // store config path
            snprintf(configPath, sizeof(configPath), "%s/%s/config.json", base_dir, entry->d_name);
            strncpy(st->emus[st->emu_count].path, configPath, STR_MAX - 1);
            st->emus[st->emu_count].path[STR_MAX - 1] = '\0';

            processConfig(configPath, st);
        }
    }

    closedir(dir);
}

//
// Updates data where necessary
//
void updateData(AppState *st)
{
    static uint32_t ticks = 0;
    uint32_t current_ticks = SDL_GetTicks();

    if (current_ticks - ticks > 1000) {
        if (battery_hasChanged(0, &st->battery_percentage))
            st->battery_changed = true;
        ticks = current_ticks;
    }

    if (st->all_changed || st->list_content_changed) {
        // free and rebuld list
        buildList(st);

        // restore position
        st->emu_list.active_pos = st->tmp_active_pos;
        st->emu_list.scroll_pos = st->tmp_scroll_pos;

        // handle scrolling if the active position is outside the visible area
        if (st->tmp_active_pos < st->tmp_scroll_pos)
            st->emu_list.scroll_pos--;
        else if (st->tmp_active_pos >= st->tmp_scroll_pos + st->emu_list.scroll_height)
            st->emu_list.scroll_pos++;

        // redraw list and footer
        st->list_changed = true;
        st->list_content_changed = false;
    }
}

//
// Update the icon for the selected Emulator
//
void updateIcon(AppState *st)
{
    for (int i = 0; i < st->emu_count; i++)
        st->emu_list.items[i].icon_ptr = st->emu_list.active_pos == i ? st->emus[i].icon_sel : st->emus[i].icon;
}

//
// Renders the header, list and footer if necessary
//
void render(AppState *st)
{

    if (st->header_changed || st->battery_changed || st->all_changed) {
        START_TIMER(tm_renderHeader);
        theme_renderHeader(screen, "EmuSort", false);
        theme_renderHeaderBattery(screen, st->battery_percentage);
        END_TIMER(tm_renderHeader);
    }

    if (st->list_changed || st->all_changed) {
        START_TIMER(tm_renderList);
        updateIcon(st);
        theme_renderList(screen, &st->emu_list);
        END_TIMER(tm_renderList);
        START_TIMER(tm_renderFooter);
        theme_renderFooter(screen);
        theme_renderStandardHint(screen, "SAVE", "CANCEL");
        theme_renderFooterStatus(screen, st->emu_list.active_pos + 1, st->emu_count);
        END_TIMER(tm_renderFooter);
    }

    // if anything changed, update the screen
    if (st->list_changed || st->header_changed || st->all_changed || st->battery_changed) {
        blitFlip();
        st->list_changed = st->header_changed = st->all_changed = st->battery_changed = false;
    }
}

//
// Update the playtime field for the emulator with the given rompath
//
bool updatePlaytime(AppState *st, const char *rompath, int playtime)
{
    for (int i = 0; i < st->emu_count; i++) {
        if (strcmp(st->emus[i].rompath, rompath) == 0) {
            st->emus[i].playtime = playtime;
            return true;
        }
    }
    return false;
}

//
// Get the playtime for each emulator from the play_activity database
//
void getPlayTimes(AppState *st)
{

    sqlite3_stmt *stmt;

    char *sql = "SELECT SUBSTR(file_path, 1, INSTR(file_path, '/') - 1) AS emulator, "
                "SUM(play_time) AS play_time_total "
                "FROM play_activity "
                "JOIN rom ON play_activity.rom_id = rom.id "
                "WHERE file_path != '' "
                "GROUP BY emulator "
                "ORDER BY play_time_total DESC;";

    play_activity_db_open();

    stmt = play_activity_db_prepare(sql);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *emulator = (const char *)sqlite3_column_text(stmt, 0);
        int playtime = sqlite3_column_int(stmt, 1);

        if (!updatePlaytime(st, emulator, playtime))
            fprintf(stderr, "Emulator not found in AppState: %s\n", emulator);
    }

    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    play_activity_db_close();
}

int main()
{
    static bool first = true;

    START_TIMER(tm_loading);
    START_TIMER(tm_init);
    AppState *st = init();
    END_TIMER(tm_init);

    if (!is_file(HIDE_SPLASH_FLAG))
        showSplashScreen(st);

    // get all emulators from the Emu directory, sort alphabetically as they would appear in MainUI
    START_TIMER(tm_get_data);
    processDirectories(EMU_DIR, st);
    qsort(st->emus, st->emu_count, sizeof(st->emus[0]), compareEmuLabelPadded);
    END_TIMER(tm_get_data);

    // get playtime for each emulator
    START_TIMER(tm_get_playtime);
    getPlayTimes(st);
    END_TIMER(tm_get_playtime);

    // main loop
    while (!quit) {
        handleKeys(st);
        updateData(st);
        render(st);

        if (first) {
            first = false;
            END_TIMER(tm_loading);
        }
    }

    // clear the screen to avoid flickering
    SDL_FillRect(video, NULL, 0);
    SDL_Flip(video);

    freeResources(st);
    return EXIT_SUCCESS;
}