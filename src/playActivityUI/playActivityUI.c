#include "./playActivityUI.h"

// one day we'll have resolution independence
#define RENDER_WIDTH 640
#define RENDER_HEIGHT 480
#define RENDER_DEPTH 32

#define NUM_ROWS 4
#define ROW_HEIGHT 360 / NUM_ROWS

static bool quit = false;

static const SDL_Color color_white = {255, 255, 255};
static const SDL_Color color_purple = {136, 97, 252};
static const SDL_Color color_grey = {117, 123, 156};
static const SDL_Color color_lightgrey = {214, 223, 246};
static const SDL_Color color_message = {104, 104, 104};
static const SDL_Rect rectPages = {620, 430, 90, 44};
static const SDL_Rect rectMileage = {484, 8, 170, 42};

typedef struct {
    SDL_Surface *video;
    SDL_Surface *screen;
    SDL_Surface *background;
    SDL_Surface *pop_bg;
    SDL_Surface *button_a;
    SDL_Surface *button_b;
    SDL_Surface *label_ok;
    SDL_Surface *label_cancel;
    TTF_Font *font40;
    TTF_Font *font30;
    TTF_Font *font18;
    TTF_Font *fontCJKRomName25;
} UIResources;

typedef struct {
    bool keys_enabled;
    bool reload_list;
    bool changed;
    bool show_raw_names;
    int current_page;
    int last_page;
    int current_page_rows;
    int selected;
    int num_pages;
    int play_time_total;
    char num_entries_str[25];
    char play_time_total_formatted[STR_MAX];
    KeyState keystate[320];
    PlayActivities *play_activities;
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

void init(AppState *st)
{
    memset(st, 0, sizeof(*st));
    st->res = malloc(sizeof(UIResources));
    if (!st->res) {
        perror("Failed to allocate memory for UIResources\n");
        exit(EXIT_FAILURE);
    }
    st->keys_enabled = true;
    st->reload_list = true;
    st->changed = true;

    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_EnableKeyRepeat(300, 50);

    TTF_Init();

    st->res->video = SDL_SetVideoMode(RENDER_WIDTH, RENDER_HEIGHT, RENDER_DEPTH, SDL_HWSURFACE);
    st->res->screen = SDL_CreateRGBSurface(SDL_HWSURFACE, RENDER_WIDTH, RENDER_HEIGHT, RENDER_DEPTH, 0, 0, 0, 0);

    st->res->background = IMG_Load("./res/background.png");
    st->res->pop_bg = IMG_Load("/mnt/SDCARD/miyoo/app/skin/pop-bg.png");
    st->res->button_a = IMG_Load("/mnt/SDCARD/miyoo/app/skin/icon-A-54.png");
    st->res->button_b = IMG_Load("/mnt/SDCARD/miyoo/app/skin/icon-B-54.png");

    st->res->font40 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 40);
    st->res->font30 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 30);
    st->res->fontCJKRomName25 = TTF_OpenFont("/customer/app/wqy-microhei.ttc", 25);
    st->res->font18 = TTF_OpenFont("/customer/app/wqy-microhei.ttc", 18);

    st->res->label_ok = TTF_RenderUTF8_Blended(st->res->font30, "OK", color_white);
    st->res->label_cancel = TTF_RenderUTF8_Blended(st->res->font30, "Cancel", color_white);
}

void free_resources(AppState *st)
{
    TTF_CloseFont(st->res->font40);
    TTF_CloseFont(st->res->font30);
    TTF_CloseFont(st->res->fontCJKRomName25);
    TTF_CloseFont(st->res->font18);
    TTF_Quit();

    SDL_FreeSurface(st->res->label_ok);
    SDL_FreeSurface(st->res->label_cancel);
    SDL_FreeSurface(st->res->background);
    SDL_FreeSurface(st->res->pop_bg);
    SDL_FreeSurface(st->res->button_a);
    SDL_FreeSurface(st->res->button_b);
    SDL_FreeSurface(st->res->screen);
    SDL_FreeSurface(st->res->video);
    SDL_Quit();

    free_play_activities(st->play_activities);
    free(st);
}

int _renderText(AppState *st, const char *text, TTF_Font *font, const SDL_Color color, const SDL_Rect *rect, bool right_align)
{
    int text_width = 0;
    SDL_Surface *text_surface = TTF_RenderUTF8_Blended(font, text, color);
    if (text_surface != NULL) {
        text_width = text_surface->w;
        SDL_Rect alignRect = right_align ? (SDL_Rect){rect->x - text_surface->w, rect->y, rect->w, rect->h} : *rect;
        SDL_BlitSurface(text_surface, NULL, st->res->screen, &alignRect);
        SDL_FreeSurface(text_surface);
    }
    return text_width;
}

int renderText(AppState *st, const char *text, TTF_Font *font, const SDL_Color color, const SDL_Rect *rect)
{
    return _renderText(st, text, font, color, rect, false);
}

int renderTextAlignRight(AppState *st, const char *text, TTF_Font *font, const SDL_Color color, const SDL_Rect *rect)
{
    return _renderText(st, text, font, color, rect, true);
}

SDL_Surface *loadRomImage(const char *image_path)
{
    SDL_Surface *img = IMG_Load(is_file(image_path) ? image_path : "/mnt/SDCARD/miyoo/app/skin/thumb-default.png");

    double sw = (double)IMG_MAX_WIDTH / img->w;
    double sh = (double)IMG_MAX_HEIGHT / img->h;
    double s = MIN(sw, sh);

    SDL_PixelFormat *ft = img->format;
    SDL_Surface *dst = SDL_CreateRGBSurface(0, (int)(s * img->w), (int)(s * img->h), ft->BitsPerPixel, ft->Rmask, ft->Gmask, ft->Bmask, ft->Amask);

    SDL_Rect src_rect = {0, 0, img->w, img->h};
    SDL_Rect dst_rect = {0, 0, dst->w, dst->h};
    SDL_SoftStretch(img, &src_rect, dst, &dst_rect);

    SDL_FreeSurface(img);

    return dst;
}

void renderPage(AppState *st)
{
    char num_str[12];
    char rom_name[STR_MAX];
    char total[25];
    char average[25];
    char plays[25];

    int num_width = 50;
    if (st->current_page >= 2)
        num_width += 20;
    if (st->current_page >= 24)
        num_width += 20;

    st->current_page_rows = 0;
    st->last_page = -1;

    if (st->last_page != st->current_page) {
        st->last_page = st->current_page;

        // if we are on the last page and it has less than NUM_ROWS entries, we might need to adjust the selected index
        if (st->current_page == st->num_pages - 1 &&
            st->selected >= (st->play_activities->count % NUM_ROWS == 0 ? NUM_ROWS : st->play_activities->count % NUM_ROWS)) {
            st->selected = (st->play_activities->count - 1) % NUM_ROWS;
            st->changed = true;
        }
    }

    SDL_Color index_color = color_purple;

    for (int row = 0; row < NUM_ROWS; row++) {
        int index = st->current_page * NUM_ROWS + row;

        if (index >= st->play_activities->count)
            break;

        st->current_page_rows++;

        PlayActivity *entry = st->play_activities->play_activity[index];
        ROM *rom = entry->rom;

        index_color = color_purple;
        if (row == st->selected) {
            index_color = color_white;
            SDL_Rect rect = {0, 65 + ROW_HEIGHT * row, RENDER_WIDTH, ROW_HEIGHT};
            SDL_FillRect(st->res->screen, &rect, SDL_MapRGB(st->res->screen->format, color_purple.r, color_purple.g, color_purple.b));
        }

        sprintf(num_str, "%d", index + 1);
        renderTextAlignRight(st, num_str, st->res->font40, index_color, &(SDL_Rect){num_width, 80 + ROW_HEIGHT * row, 50, 39});

        SDL_Surface *romImage = loadRomImage(rom->image_path);
        SDL_Rect rectRomImage = {num_width + 10 + (80 - romImage->w) / 2, 70 + ROW_HEIGHT * row, 80, 80};
        SDL_BlitSurface(romImage, NULL, st->res->screen, &rectRomImage);
        SDL_FreeSurface(romImage);

        if (st->show_raw_names)
            strncpy(rom_name, rom->name, STR_MAX - 1);
        else
            file_cleanName(rom_name, rom->name);

        renderText(st, rom_name, includeCJK(rom_name) ? st->res->fontCJKRomName25 : st->res->font30, color_white, &(SDL_Rect){num_width + 100, 75 + ROW_HEIGHT * row, 400, 40});

        str_serializeTime(total, entry->play_time_total);
        str_serializeTime(average, entry->play_time_average);
        snprintf(plays, 24, "%d", entry->play_count);

        const char *details[] = {"TOTAL ", total, "  AVG ", average, "  PLAYS ", plays};
        SDL_Rect detailsRect = {num_width + 100, 115 + ROW_HEIGHT * row, 400, 40};
        for (int i = 0; i < 6; i++) {
            detailsRect.x += renderText(st, details[i], st->res->font18, i % 2 == 0 ? (row == st->selected ? color_white : color_grey) : color_lightgrey, &detailsRect);
        }
    }
}

//
// Create a textbox surface with the provided message
// The message will be split into lines at newline characters
// Lines longer than max_width will be truncated
//
SDL_Surface *textboxSurface(const char *message, TTF_Font *font,
                            SDL_Color fg, int max_width)
{
    SDL_Surface *lines[6];
    int line_count = 0;
    int line_width = 0;
    int line_height = 1.2 * TTF_FontLineSkip(font);

    char *token = NULL;
    char *delim = "\n";
    char s[STR_MAX];
    strcpy(s, message);

    token = strtok(s, delim);
    while (token != NULL) {
        SDL_Surface *line = TTF_RenderUTF8_Blended(font, token, fg);
        int currentWidth = line->w;

        while (currentWidth > max_width) {
            // cut characters until the line fits
            // i promise it's fast
            SDL_FreeSurface(line);
            size_t tokenLength = strlen(token);
            token[tokenLength - 1] = '\0';
            line = TTF_RenderUTF8_Blended(font, token, fg);
            currentWidth = line->w;
        }

        lines[line_count] = line;
        SDL_SetAlpha(lines[line_count], 0, 0);

        if (lines[line_count]->w > line_width) {
            line_width = lines[line_count]->w;
        }

        line_count++;
        token = strtok(NULL, delim);
    }

    SDL_Surface *textbox = SDL_CreateRGBSurface(
        0, line_width, line_height * line_count, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    SDL_FillRect(textbox, NULL, textbox->format->colorkey);

    SDL_Rect line_rect = {0, 0};

    for (int i = 0; i < line_count; i++) {
        line_rect.x = (line_width - lines[i]->w) / 2;
        line_rect.y = line_height * i + (line_height - lines[i]->h) / 2;
        SDL_BlitSurface(lines[i], NULL, textbox, &line_rect);
        SDL_FreeSurface(lines[i]);
    }

    return textbox;
}

//
// Display a confirmation dialog with the provided message
// Stolen from theme_renderDialog but without theming
//
int confirmDialog(AppState *st, const char *message)
{
    SDL_Surface *transparent_bg = SDL_CreateRGBSurface(
        0, RENDER_WIDTH, RENDER_HEIGHT, RENDER_DEPTH, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    SDL_FillRect(transparent_bg, NULL, 0xBE000000);
    SDL_BlitSurface(transparent_bg, NULL, st->res->screen, NULL);
    SDL_FreeSurface(transparent_bg);

    SDL_Rect center_rect = {RENDER_WIDTH / 2 - st->res->pop_bg->w / 2, RENDER_HEIGHT / 2 - st->res->pop_bg->h / 2};

    SDL_BlitSurface(st->res->pop_bg, NULL, st->res->screen, &center_rect);

    SDL_Surface *title = TTF_RenderUTF8_Blended(st->res->font40, "Delete entry", color_white);
    if (title) {
        SDL_Rect title_rect = {RENDER_WIDTH / 2 - title->w / 2, center_rect.y + 25 - title->h / 2};
        SDL_BlitSurface(title, NULL, st->res->screen, &title_rect);
        SDL_FreeSurface(title);
    }
    SDL_Surface *textbox = textboxSurface(message, st->res->font30, color_message, 550);
    if (textbox) {
        SDL_Rect textbox_rect = {RENDER_WIDTH / 2 - textbox->w / 2, center_rect.y + 160 - textbox->h / 2};
        SDL_BlitSurface(textbox, NULL, st->res->screen, &textbox_rect);
        SDL_FreeSurface(textbox);
    }
    SDL_Rect hint_rect = {center_rect.x + st->res->pop_bg->w - 30, center_rect.y + st->res->pop_bg->h - 60};

    hint_rect.x -= st->res->button_a->w + 5;
    hint_rect.x -= st->res->label_ok->w + 30;

    hint_rect.x -= st->res->button_b->w + 5;
    hint_rect.x -= st->res->label_cancel->w + 30;

    SDL_Rect button_a_rect = {hint_rect.x, hint_rect.y - st->res->button_a->h / 2};
    hint_rect.x += st->res->button_a->w + 5;
    SDL_BlitSurface(st->res->button_a, NULL, st->res->screen, &button_a_rect);

    SDL_Rect label_ok_rect = {hint_rect.x, hint_rect.y - st->res->label_ok->h / 2};
    hint_rect.x += st->res->label_ok->w + 30;
    SDL_BlitSurface(st->res->label_ok, NULL, st->res->screen, &label_ok_rect);

    SDL_Rect button_b_rect = {hint_rect.x, hint_rect.y - st->res->button_b->h / 2};
    hint_rect.x += st->res->button_b->w + 5;
    SDL_BlitSurface(st->res->button_b, NULL, st->res->screen, &button_b_rect);

    SDL_Rect label_cancel_rect = {hint_rect.x, hint_rect.y - st->res->label_cancel->h / 2};
    hint_rect.x += st->res->label_cancel->w + 30;
    SDL_BlitSurface(st->res->label_cancel, NULL, st->res->screen, &label_cancel_rect);

    SDL_BlitSurface(st->res->screen, NULL, st->res->video, NULL);
    SDL_Flip(st->res->video);

    // wait for user input
    int result = 0;
    bool confirm_quit = false;

    while (!confirm_quit) {
        if (updateKeystate(st->keystate, &confirm_quit, true, NULL)) {
            if (st->keystate[SW_BTN_A] == PRESSED) {
                confirm_quit = true;
                result = 1;
            }
            else if (st->keystate[SW_BTN_B] == PRESSED) {
                result = 0;
                confirm_quit = true;
            }
        }
    }

    st->changed = true;
    return result;
}

//
// Delete (hide) the play activity for the provided index
//
void deleteIndex(int index, AppState *st)
{
    printf_debug("hiding play activity for %s\n",
                 st->play_activities->play_activity[index]->rom->name);
    play_activity_hide(st->play_activities->play_activity[index]->rom->id);
    st->play_activities->count--;

    if (st->play_activities->count > 0) {
        // select the entry before the deleted one
        st->selected = (index - 1) % NUM_ROWS;
        st->current_page = (index - 1) / NUM_ROWS;
        if (st->selected < 0)
            st->selected = 0;
    }
    else {
        // we deleted the last entry
        st->selected = 0;
        st->current_page = 0;
    }

    st->reload_list = true;
    st->changed = true;
}

void handleKeys(AppState *st)
{
    if (st->keys_enabled && updateKeystate(st->keystate, &quit, true, NULL)) {
        if (st->keystate[SW_BTN_B] == PRESSED) // quit
            quit = true;

        if (st->keystate[SW_BTN_RIGHT] >= PRESSED) { // next page
            if (st->current_page < st->num_pages - 1) {
                st->current_page++;
                st->changed = true;
            }
        }
        if (st->keystate[SW_BTN_LEFT] >= PRESSED) { // previous page
            if (st->current_page > 0) {
                st->current_page--;
                st->changed = true;
            }
        }
        if (st->keystate[SW_BTN_Y] == PRESSED) { // toggle show raw names
            st->show_raw_names = !st->show_raw_names;
            st->changed = true;
        }
        if (st->keystate[SW_BTN_DOWN] >= PRESSED) { // select next entry
            if (st->selected == NUM_ROWS - 1 && st->current_page < st->num_pages - 1) {
                // end of page, go to next page
                st->current_page++;
                st->selected = 0;
            }
            else if (st->selected < NUM_ROWS - 1 && st->selected < st->current_page_rows - 1)
                st->selected++;
            st->changed = true;
        }
        if (st->keystate[SW_BTN_UP] >= PRESSED) { // select previous entry
            if (st->selected == 0 && st->current_page > 0) {
                // start of page, go to previous page
                st->current_page--;
                st->selected = NUM_ROWS - 1;
            }
            else if (st->selected > 0)
                st->selected--;
            st->changed = true;
        }
        if (st->keystate[SW_BTN_X] == PRESSED && st->play_activities->count > 0) { // delete entry
            st->keys_enabled = false;
            int index = st->current_page * NUM_ROWS + st->selected;
            char message[STR_MAX];
            snprintf(message, STR_MAX, "Delete the play activity for\n%s",
                     st->play_activities->play_activity[index]->rom->name);
            if (confirmDialog(st, message))
                deleteIndex(index, st);
            st->keys_enabled = true;
        }
    }
}

//
// Reload the list of play activities and update the total play time
//
void reloadList(AppState *st)
{
    SDL_BlitSurface(st->res->background, NULL, st->res->screen, NULL);

    st->play_activities = play_activity_find_all(false);
    printf_debug("found %d roms\n", st->play_activities->count);

    st->num_pages = (int)ceil((double)st->play_activities->count / (double)NUM_ROWS);

    st->play_time_total = st->play_activities->play_time_total;

    str_serializeTime(st->play_time_total_formatted, st->play_time_total);
    renderText(st, st->play_time_total_formatted, st->res->font30, color_white, &rectMileage);

    st->reload_list = false;
}

int main(int argc, char *argv[])
{
    log_setName("playActivityUI");

    AppState *st = malloc(sizeof(AppState));
    if (!st) {
        perror("Failed to allocate memory for AppState\n");
        return EXIT_FAILURE;
    }

    init(st);

    while (!quit) {

        handleKeys(st);

        if (st->reload_list)
            reloadList(st);

        if (!st->changed)
            continue;

        SDL_BlitSurface(st->res->background, NULL, st->res->screen, NULL);

        renderText(st, st->play_time_total_formatted, st->res->font30, color_white, &rectMileage);

        renderPage(st);

        sprintf(st->num_entries_str, "%d/%d",
                st->current_page * NUM_ROWS + st->selected + 1, st->play_activities->count);

        renderTextAlignRight(st, st->num_entries_str, st->res->font30, color_white, &rectPages);

        st->changed = false;

        SDL_BlitSurface(st->res->screen, NULL, st->res->video, NULL);
        SDL_Flip(st->res->video);
    }

    free_resources(st);

    return EXIT_SUCCESS;
}
