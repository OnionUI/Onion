#include "./playActivityUI.h"
#include <pthread.h>

#define RENDER_WIDTH 640
#define RENDER_HEIGHT 480
#define RENDER_DEPTH 32

#define NUM_ROWS 4
#define ROW_HEIGHT 360 / NUM_ROWS

static bool quit = false;
static bool keys_enabled = true;
KeyState keystate[320] = {(KeyState)0};

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

static SDL_Surface *background;
static SDL_Surface *pop_bg;
static SDL_Surface *button_a;
static SDL_Surface *button_b;

static TTF_Font *font40;
static TTF_Font *font30;
static TTF_Font *fontCJKRomName25;
static TTF_Font *font18;

static SDL_Surface *label_ok;
static SDL_Surface *label_cancel;

static PlayActivities *play_activities;

static SDL_Color color_white = {255, 255, 255};
static SDL_Color color_purple = {136, 97, 252};
static SDL_Color color_grey = {117, 123, 156};
static SDL_Color color_lightgrey = {214, 223, 246};
static SDL_Color color_message = {0x68, 0x68, 0x68};

static bool show_raw_names = false;
static int selected = 0;
static int current_page = 0;
static int current_page_rows = 0;
static bool changed = true;
static bool reload_list = true;

void init(void)
{
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_EnableKeyRepeat(300, 50);
    TTF_Init();

    video = SDL_SetVideoMode(RENDER_WIDTH, RENDER_HEIGHT, RENDER_DEPTH, SDL_HWSURFACE);
    screen = SDL_CreateRGBSurface(SDL_HWSURFACE, RENDER_WIDTH, RENDER_HEIGHT, RENDER_DEPTH, 0, 0, 0, 0);

    background = IMG_Load("./res/background.png");
    pop_bg = IMG_Load("/mnt/SDCARD/miyoo/app/skin/pop-bg.png");
    button_a = IMG_Load("/mnt/SDCARD/miyoo/app/skin/icon-A-54.png");
    button_b = IMG_Load("/mnt/SDCARD/miyoo/app/skin/icon-B-54.png");

    font40 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 40);
    font30 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 30);
    fontCJKRomName25 = TTF_OpenFont("/customer/app/wqy-microhei.ttc", 25);
    font18 = TTF_OpenFont("/customer/app/wqy-microhei.ttc", 18);

    label_ok = TTF_RenderUTF8_Blended(font30, "OK", color_white);
    label_cancel = TTF_RenderUTF8_Blended(font30, "Cancel", color_white);
}

void free_resources(void)
{
    TTF_CloseFont(font40);
    TTF_CloseFont(font30);
    TTF_CloseFont(fontCJKRomName25);
    TTF_CloseFont(font18);

    TTF_Quit();

    SDL_FreeSurface(label_ok);
    SDL_FreeSurface(label_cancel);
    SDL_FreeSurface(background);
    SDL_FreeSurface(pop_bg);
    SDL_FreeSurface(button_a);
    SDL_FreeSurface(button_b);

    SDL_FreeSurface(screen);
    SDL_FreeSurface(video);
    SDL_Quit();

    free_play_activities(play_activities);
}

int _renderText(const char *text, TTF_Font *font, SDL_Color color, SDL_Rect *rect, bool right_align)
{
    int text_width = 0;
    SDL_Surface *textSurface = TTF_RenderUTF8_Blended(font, text, color);
    if (textSurface != NULL) {
        text_width = textSurface->w;
        if (right_align)
            SDL_BlitSurface(textSurface, NULL, screen, &(SDL_Rect){rect->x - textSurface->w, rect->y, rect->w, rect->h});
        else
            SDL_BlitSurface(textSurface, NULL, screen, rect);
        SDL_FreeSurface(textSurface);
    }
    return text_width;
}

int renderText(const char *text, TTF_Font *font, SDL_Color color, SDL_Rect *rect)
{
    return _renderText(text, font, color, rect, false);
}

int renderTextAlignRight(const char *text, TTF_Font *font, SDL_Color color, SDL_Rect *rect)
{
    return _renderText(text, font, color, rect, true);
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

void renderPage(int current_page, int num_pages)
{
    char num_str[12];
    char rom_name[STR_MAX];
    char total[25];
    char average[25];
    char plays[25];

    int num_width = 50;
    if (current_page >= 2)
        num_width += 20;
    if (current_page >= 24)
        num_width += 20;

    current_page_rows = 0;
    static int last_page = -1;

    if (last_page != current_page) {
        last_page = current_page;

        // if we are on the last page and it has less than NUM_ROWS, we might need to adjust the selected index
        if (current_page == num_pages - 1 && selected >= (play_activities->count % NUM_ROWS == 0 ? NUM_ROWS : play_activities->count % NUM_ROWS))
            selected = (play_activities->count - 1) % NUM_ROWS;
    }

    SDL_Color index_color = color_purple;

    for (int row = 0; row < NUM_ROWS; row++) {
        int index = current_page * NUM_ROWS + row;

        if (index >= play_activities->count)
            break;

        current_page_rows++;

        PlayActivity *entry = play_activities->play_activity[index];
        ROM *rom = entry->rom;

        index_color = color_purple;
        if (row == selected) {
            index_color = color_white;
            SDL_Rect rect = {0, 65 + ROW_HEIGHT * row, RENDER_WIDTH, ROW_HEIGHT};
            SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, color_purple.r, color_purple.g, color_purple.b));
        }

        sprintf(num_str, "%d", index + 1);
        renderTextAlignRight(num_str, font40, index_color, &(SDL_Rect){num_width, 80 + ROW_HEIGHT * row, 50, 39});

        SDL_Surface *romImage = loadRomImage(rom->image_path);
        SDL_Rect rectRomImage = {num_width + 10 + (80 - romImage->w) / 2, 70 + ROW_HEIGHT * row, 80, 80};
        SDL_BlitSurface(romImage, NULL, screen, &rectRomImage);
        SDL_FreeSurface(romImage);

        if (show_raw_names)
            strncpy(rom_name, rom->name, STR_MAX - 1);
        else
            file_cleanName(rom_name, rom->name);
        renderText(rom_name, includeCJK(rom_name) ? fontCJKRomName25 : font30, color_white, &(SDL_Rect){num_width + 100, 75 + ROW_HEIGHT * row, 400, 40});

        str_serializeTime(total, entry->play_time_total);
        str_serializeTime(average, entry->play_time_average);
        snprintf(plays, 24, "%d", entry->play_count);

        const char *details[] = {"TOTAL ", total, "  AVG ", average, "  PLAYS ", plays};
        SDL_Rect detailsRect = {num_width + 100, 115 + ROW_HEIGHT * row, 400, 40};
        for (int i = 0; i < 6; i++) {
            detailsRect.x += renderText(details[i], font18, i % 2 == 0 ? (row == selected ? color_white : color_grey) : color_lightgrey, &detailsRect);
        }
    }
}

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

int confirmDialog(const char *message)
{
    SDL_Surface *transparent_bg = SDL_CreateRGBSurface(
        0, RENDER_WIDTH, RENDER_HEIGHT, RENDER_DEPTH, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    SDL_FillRect(transparent_bg, NULL, 0xBE000000);
    SDL_BlitSurface(transparent_bg, NULL, screen, NULL);
    SDL_FreeSurface(transparent_bg);

    SDL_Rect center_rect = {RENDER_WIDTH / 2 - pop_bg->w / 2, RENDER_HEIGHT / 2 - pop_bg->h / 2};

    SDL_BlitSurface(pop_bg, NULL, screen, &center_rect);

    SDL_Surface *title = TTF_RenderUTF8_Blended(font40, "Delete entry", color_white);
    if (title) {
        SDL_Rect title_rect = {RENDER_WIDTH / 2 - title->w / 2, center_rect.y + 25 - title->h / 2};
        SDL_BlitSurface(title, NULL, screen, &title_rect);
        SDL_FreeSurface(title);
    }
    SDL_Surface *textbox = textboxSurface(message, font30, color_message, 550);
    if (textbox) {
        SDL_Rect textbox_rect = {RENDER_WIDTH / 2 - textbox->w / 2, center_rect.y + 160 - textbox->h / 2};
        SDL_BlitSurface(textbox, NULL, screen, &textbox_rect);
        SDL_FreeSurface(textbox);
    }
    SDL_Rect hint_rect = {center_rect.x + pop_bg->w - 30, center_rect.y + pop_bg->h - 60};

    hint_rect.x -= button_a->w + 5;
    hint_rect.x -= label_ok->w + 30;

    hint_rect.x -= button_b->w + 5;
    hint_rect.x -= label_cancel->w + 30;

    SDL_Rect button_a_rect = {hint_rect.x, hint_rect.y - button_a->h / 2};
    hint_rect.x += button_a->w + 5;
    SDL_BlitSurface(button_a, NULL, screen, &button_a_rect);

    SDL_Rect label_ok_rect = {hint_rect.x, hint_rect.y - label_ok->h / 2};
    hint_rect.x += label_ok->w + 30;
    SDL_BlitSurface(label_ok, NULL, screen, &label_ok_rect);

    SDL_Rect button_b_rect = {hint_rect.x, hint_rect.y - button_b->h / 2};
    hint_rect.x += button_b->w + 5;
    SDL_BlitSurface(button_b, NULL, screen, &button_b_rect);

    SDL_Rect label_cancel_rect = {hint_rect.x, hint_rect.y - label_cancel->h / 2};
    hint_rect.x += label_cancel->w + 30;
    SDL_BlitSurface(label_cancel, NULL, screen, &label_cancel_rect);

    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);

    // wait for user input
    int result = 0;
    bool confirm_quit = false;

    while (!confirm_quit) {
        if (updateKeystate(keystate, &confirm_quit, true, NULL)) {
            if (keystate[SW_BTN_A] == PRESSED) {
                confirm_quit = true;
                result = 1;
            }
            else if (keystate[SW_BTN_B] == PRESSED) {
                result = 0;
                confirm_quit = true;
            }
        }
    }

    keys_enabled = true;
    changed = true;
    return result;
}

//
// Delete the play activity for the selected index
// Maybe we shouldn't delete but just mark it as deleted/hidden?
//
void deleteIndex(int index)
{
    log_debug("hiding play activity for %s\n", play_activities->play_activity[index]->rom->name);
    play_activity_hide(play_activities->play_activity[index]->rom->id);
    play_activities->count--;

    if (play_activities->count > 0) {
        // select the entry before the deleted one
        selected = (index - 1) % NUM_ROWS;
        current_page = (index - 1) / NUM_ROWS;
        if (selected < 0)
            selected = 0;
    }
    else {
        // we deleted the last entry
        selected = 0;
        current_page = 0;
    }

    reload_list = true;
    changed = true;
}

int main(int argc, char *argv[])
{
    log_setName("playActivityUI");

    init();

    SDL_Rect rectPages = {620, 430, 90, 44};
    SDL_Rect rectMileage = {484, 8, 170, 42};

    int num_pages, play_time_total;
    char num_pages_str[25];
    char play_time_total_formatted[STR_MAX];
    while (!quit) {
        if (reload_list) {
            SDL_BlitSurface(background, NULL, screen, NULL);

            play_activities = play_activity_find_all(false);
            printf_debug("found %d roms\n", play_activities->count);

            num_pages = (int)ceil((double)play_activities->count / (double)NUM_ROWS);

            renderPage(current_page, num_pages);

            play_time_total = play_activities->play_time_total;

            str_serializeTime(play_time_total_formatted, play_time_total);
            renderText(play_time_total_formatted, font30, color_white, &rectMileage);

            SDL_BlitSurface(screen, NULL, video, NULL);
            SDL_Flip(video);
            reload_list = false;
        }

        if (keys_enabled && updateKeystate(keystate, &quit, true, NULL)) {
            if (keystate[SW_BTN_B] == PRESSED)
                quit = true;
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
            if (keystate[SW_BTN_Y] == PRESSED) {
                show_raw_names = !show_raw_names;
                changed = true;
            }
            if (keystate[SW_BTN_DOWN] >= PRESSED) {
                if (selected == NUM_ROWS - 1 && current_page < num_pages - 1) {
                    // next page
                    current_page++;
                    selected = 0;
                }
                else if (selected < NUM_ROWS - 1 && selected < current_page_rows - 1)
                    selected++;
                changed = true;
            }
            if (keystate[SW_BTN_UP] >= PRESSED) {
                if (selected == 0 && current_page > 0) {
                    // back to previous page
                    current_page--;
                    selected = NUM_ROWS - 1;
                }
                else if (selected > 0)
                    selected--;
                changed = true;
            }
            if (keystate[SW_BTN_X] == PRESSED && play_activities->count > 0) {
                // delete selected
                int index = current_page * NUM_ROWS + selected;
                char *message = malloc(STR_MAX);
                if (message == NULL) {
                    perror("malloc");
                    continue;
                }
                snprintf(message, STR_MAX, "Delete the play activity for\n%s", play_activities->play_activity[index]->rom->name);
                if (confirmDialog(message))
                    deleteIndex(index);

                free(message);
            }
        }

        if (!changed)
            continue;

        SDL_BlitSurface(background, NULL, screen, NULL);

        sprintf(num_pages_str, "%d/%d", current_page * NUM_ROWS + selected + 1, play_activities->count);
        renderTextAlignRight(num_pages_str, font30, color_white, &rectPages);

        renderText(play_time_total_formatted, font30, color_white, &rectMileage);

        renderPage(current_page, num_pages);

        SDL_BlitSurface(screen, NULL, video, NULL);
        SDL_Flip(video);
        changed = false;
    }

    free_resources();

    return EXIT_SUCCESS;
}
