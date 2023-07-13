#include "./playActivityUI.h"

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

static SDL_Surface *background;

static TTF_Font *font40;
static TTF_Font *font30;
static TTF_Font *fontRomName25;
static TTF_Font *font18;

static PlayActivities *play_activities;

static SDL_Color color_white = {255, 255, 255};
static SDL_Color color_purple = {136, 97, 252};

static bool show_raw_names = false;

void init(void)
{
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_EnableKeyRepeat(300, 50);
    TTF_Init();

    video = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE);
    screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);

    background = IMG_Load("./res/background.png");

    font40 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 40);
    font30 = TTF_OpenFont("/customer/app/Exo-2-Bold-Italic.ttf", 30);
    fontRomName25 = TTF_OpenFont("/customer/app/wqy-microhei.ttc", 25);
    font18 = TTF_OpenFont("/customer/app/wqy-microhei.ttc", 18);
}

void free_resources(void)
{
    TTF_CloseFont(font40);
    TTF_CloseFont(font30);
    TTF_CloseFont(fontRomName25);
    TTF_CloseFont(font18);

    TTF_Quit();

    SDL_FreeSurface(background);

    SDL_FreeSurface(screen);
    SDL_FreeSurface(video);
    SDL_Quit();

    free_play_activities(play_activities);
}

void _renderText(const char *text, TTF_Font *font, SDL_Color color, SDL_Rect rect, bool right_align)
{
    SDL_Surface *textSurface = TTF_RenderUTF8_Blended(font, text, color);
    if (textSurface != NULL) {
        if (right_align)
            rect.x -= textSurface->w;
        SDL_BlitSurface(textSurface, NULL, screen, &rect);
        SDL_FreeSurface(textSurface);
    }
}

void renderText(const char *text, TTF_Font *font, SDL_Color color, SDL_Rect rect)
{
    _renderText(text, font, color, rect, false);
}

void renderTextAlignRight(const char *text, TTF_Font *font, SDL_Color color, SDL_Rect rect)
{
    _renderText(text, font, color, rect, true);
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

void renderPage(int current_page)
{
    char num_str[12];
    char rom_name[STR_MAX];
    char total[25];
    char average[25];
    char details[STR_MAX];

    int num_width = 50;
    if (current_page >= 2)
        num_width += 20;
    if (current_page >= 24)
        num_width += 20;

    for (int row = 0; row < 4; row++) {
        int index = current_page * 4 + row;

        if (index >= play_activities->count)
            break;

        PlayActivity *entry = play_activities->play_activity[index];
        ROM *rom = entry->rom;

        sprintf(num_str, "%d", index + 1);
        renderTextAlignRight(num_str, font40, color_purple, (SDL_Rect){num_width, 80 + 90 * row, 50, 39});

        SDL_Surface *romImage = loadRomImage(rom->image_path);
        SDL_Rect rectRomImage = {num_width + 10 + (80 - romImage->w) / 2, 70 + 90 * row, 80, 80};
        SDL_BlitSurface(romImage, NULL, screen, &rectRomImage);
        SDL_FreeSurface(romImage);

        if (show_raw_names) {
            strncpy(rom_name, rom->name, STR_MAX - 1);
        }
        else {
            str_removeParentheses(rom_name, str_replace(file_removeExtension(rom->name), "_", " "));
        }
        renderText(rom_name, font30, color_white, (SDL_Rect){num_width + 100, 75 + 90 * row, 400, 40});

        str_serializeTime(total, entry->play_time_total);
        str_serializeTime(average, entry->play_time_average);
        snprintf(details, 100, "Total: %s   Avg: %s   Plays: %d", total, average, entry->play_count);
        renderText(details, font18, color_white, (SDL_Rect){num_width + 100, 115 + 90 * row, 400, 40});
    }
}

int main(int argc, char *argv[])
{
    log_setName("playActivityUI");

    init();

    SDL_BlitSurface(background, NULL, screen, NULL);

    SDL_Rect rectPages = {620, 430, 90, 44};
    SDL_Rect rectMileage = {484, 8, 170, 42};

    play_activities = play_activity_find_all();
    printf_debug("found %d roms\n", play_activities->count);

    int num_pages = (int)ceil((double)play_activities->count / (double)4);
    int current_page = 0;

    renderPage(current_page);

    char num_pages_str[25];
    sprintf(num_pages_str, "%d/%d", current_page + 1, num_pages);
    renderTextAlignRight(num_pages_str, font30, color_white, rectPages);

    int play_time_total = play_activities->play_time_total;
    char play_time_total_formatted[STR_MAX];
    str_serializeTime(play_time_total_formatted, play_time_total);
    renderText(play_time_total_formatted, font30, color_white, rectMileage);

    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);

    bool changed;
    KeyState keystate[320] = {(KeyState)0};

    while (!quit) {
        changed = false;

        if (updateKeystate(keystate, &quit, true, NULL)) {
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
        }

        if (!changed)
            continue;

        SDL_BlitSurface(background, NULL, screen, NULL);

        sprintf(num_pages_str, "%d/%d", current_page + 1, num_pages);
        renderTextAlignRight(num_pages_str, font30, color_white, rectPages);

        renderText(play_time_total_formatted, font30, color_white, rectMileage);

        renderPage(current_page);

        SDL_BlitSurface(screen, NULL, video, NULL);
        SDL_Flip(video);
    }

    free_resources();

    return EXIT_SUCCESS;
}
