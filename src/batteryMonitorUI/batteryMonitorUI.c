#include "./batteryMonitorUI.h"
#include "system/device_model.h"

#include "../batmon/batmonDB.h"

static bool quit = false;
static int current_zoom = 1;
static int current_page = 0;
static int current_index;

// Zoom level
static int segment_duration;

static char label[4][4];
static SDL_Color color_white = {255, 255, 255};
static SDL_Color color_pastel_blue = {89, 167, 255};
static int graph_max_size = GRAPH_MAX_FULL_PAGES * GRAPH_DISPLAY_SIZE_X;
static int estimation_line_size = 0;
static int begining_session_index;
static char session_duration[10];
static char current_percentage[10];
static char session_left[10];
static char session_best[10];

typedef struct {
    int pixel_height;
    bool is_charging;
    bool is_estimated;
} graph_spot;

graph_spot graphic[GRAPH_MAX_FULL_PAGES * GRAPH_DISPLAY_SIZE_X];
//int graphic_size;

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
static SDL_Surface *waiting_screen;
static SDL_Surface *right_arrow;
static SDL_Surface *left_arrow;
static SDL_Surface *end_graph;
static TTF_Font *font_Arkhip;

void init(void)
{
    log_setName("batteryMonitorUI");
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_EnableKeyRepeat(300, 50);
    TTF_Init();

    video = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE);
    screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);
    waiting_screen = IMG_Load("./res/waiting_screen.png");
    background = IMG_Load("./res/background.png");
    right_arrow = IMG_Load("./res/right_arrow.png");
    left_arrow = IMG_Load("./res/left_arrow.png");
    end_graph = IMG_Load("./res/end.png");

    font_Arkhip = TTF_OpenFont("./res/Arkhip_font.ttf", 15);
}

void free_resources(void)
{
    TTF_CloseFont(font_Arkhip);
    TTF_Quit();

    SDL_FreeSurface(waiting_screen);
    SDL_FreeSurface(background);
    SDL_FreeSurface(right_arrow);
    SDL_FreeSurface(left_arrow);
    SDL_FreeSurface(end_graph);

    SDL_FreeSurface(screen);
    SDL_FreeSurface(video);
    SDL_Quit();
}

void secondsToHoursMinutes(int seconds, char *output)
{
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    sprintf(output, "%dh%02d", hours, minutes);
}

void drawLine(int x1, int y1, int x2, int y2, Uint32 color)
{
    int dx, dy, sx, sy, err, e2;

    dx = abs(x2 - x1);
    dy = abs(y2 - y1);

    if (x1 < x2) {
        sx = 1;
    }
    else {
        sx = -1;
    }

    if (y1 < y2) {
        sy = 1;
    }
    else {
        sy = -1;
    }

    err = dx - dy;

    while (1) {
        SDL_Rect pixel = {x1, y1, 1, 1};
        SDL_FillRect(screen, &pixel, color);

        if (x1 == x2 && y1 == y2) {
            break;
        }

        e2 = 2 * err;

        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }

        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
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

void switch_zoom_profile(int segment_duration)
{

    switch (segment_duration) {
    case 7200:
        // A segemmt is 120 minutes
        sprintf(label[0], "%s", "4h");
        sprintf(label[1], "%s", "8h");
        sprintf(label[2], "%s", "12h");
        sprintf(label[3], "%s", "16h");
        break;
    case 3600:
        // A segemmt is 60 minutes
        sprintf(label[0], "%s", "2h");
        sprintf(label[1], "%s", "4h");
        sprintf(label[2], "%s", "6h");
        sprintf(label[3], "%s", "8h");
        break;
    case 1800:
        // A segemmt is 30 minutes
        sprintf(label[0], "%s", "1h");
        sprintf(label[1], "%s", "2h");
        sprintf(label[2], "%s", "3h");
        sprintf(label[3], "%s", "4h");
        break;

    default:
        sprintf(label[0], "%s", "");
        sprintf(label[1], "%s", "");
        sprintf(label[2], "%s", "");
        sprintf(label[3], "%s", "");
        break;
    }
}

void render_waiting_screen()
{
    SDL_BlitSurface(waiting_screen, NULL, screen, NULL);
    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);
    sleep(1.5);
}

int battery_to_pixel(int battery_perc)
{
    // Converts a battery percentage to a pixel coordinate
    int y = (int)((GRAPH_DISPLAY_SIZE_Y * battery_perc) / 100) + GRAPH_DISPLAY_START_Y;

    if ((y < 0) || (y > 480)) {
        return -1;
    }
    else {
        return y;
    }
}

int duration_to_pixel(int duration)
{
    // Convert a duration to a number of pixel
    // (At a scale of zoom = 1:1, one segment = 30mn)
    // 270 mn total, 16200 seconds
    return (int)((GRAPH_DISPLAY_SIZE_X * duration) / GRAPH_DISPLAY_DURATION);
}

void compute_graph(void)
{
    int total_duration = 0;
    int previous_index = graph_max_size - 1;
    bool is_estimation_computed = false;

    secondsToHoursMinutes(get_best_session_time(), session_best);

    if (open_battery_log_db() == 1) {
        if (bat_log_db != NULL) {
            const char *sql = "SELECT * FROM bat_activity WHERE device_serial = ? ORDER BY id DESC;";
            sqlite3_stmt *stmt;
            int rc = sqlite3_prepare_v2(bat_log_db, sql, -1, &stmt, 0);

            if (rc == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1, DEVICE_SN, -1, SQLITE_STATIC);
                bool b_quit = false;

                while ((sqlite3_step(stmt) == SQLITE_ROW) && (!b_quit)) {

                    int bat_perc = sqlite3_column_int(stmt, 2);
                    int duration = sqlite3_column_int(stmt, 3);
                    bool is_charging = sqlite3_column_int(stmt, 4);

                    if (total_duration == 0) {
                        sprintf(current_percentage, "%d%%", bat_perc);
                    }

                    current_index = (graph_max_size - 1) - duration_to_pixel(total_duration);
                    graphic[current_index].is_charging = is_charging;

                    if (bat_perc > 100)
                        bat_perc = 100;

                    int current_value = battery_to_pixel(bat_perc);

                    graphic[current_index].pixel_height = current_value;

                    int segment = previous_index - current_index;
                    if (segment > 1) {
                        for (int i = 1; i < segment; i++) {
                            //  graphic[current_index+i].pixel_height = graphic[current_index].pixel_height + (int)(((graphic[previous_index].pixel_height-current_value)/segment-1)*i);
                            graphic[current_index + i].pixel_height = graphic[previous_index].pixel_height;
                            graphic[current_index + i].is_charging = graphic[previous_index].is_charging;
                        }
                    }

                    if ((is_charging) && (!is_estimation_computed)) {
                        secondsToHoursMinutes(total_duration, session_duration);
                        if (previous_index < (graph_max_size - duration_to_pixel(GRAPH_MIN_SESSION_FOR_ESTIMATION))) {
                            float slope = (float)(graphic[graph_max_size - 1].pixel_height - graphic[previous_index].pixel_height) / (float)(graph_max_size - 1 - previous_index);

                            if (slope < 0) {

                                estimation_line_size = (int)-(graphic[graph_max_size - 1].pixel_height - GRAPH_DISPLAY_START_Y) / slope;

                                int estimated_playtime = (int)(estimation_line_size)*GRAPH_DISPLAY_DURATION / GRAPH_DISPLAY_SIZE_X;

                                if (estimated_playtime < GRAPH_MAX_PLAUSIBLE_ESTIMATION) {
                                    secondsToHoursMinutes(estimated_playtime, session_left);
                                    // shift of the existing logs to make room for the estimation line
                                    int room_to_make = estimation_line_size + GRAPH_ESTIMATED_LINE_GAP;
                                    if (current_index - room_to_make >= 0) {
                                        for (int i = current_index; i < graph_max_size; i++) {
                                            graphic[i - room_to_make].pixel_height = graphic[i].pixel_height;
                                            graphic[i - room_to_make].is_charging = graphic[i].is_charging;
                                            graphic[i].pixel_height = 0;
                                            graphic[i].is_charging = false;
                                        }
                                    }
                                    total_duration += estimated_playtime + (int)(GRAPH_ESTIMATED_LINE_GAP)*GRAPH_DISPLAY_DURATION / GRAPH_DISPLAY_SIZE_X;
                                    current_index -= room_to_make;
                                    previous_index -= room_to_make;
                                    begining_session_index = previous_index;
                                    for (int x = (graph_max_size - room_to_make); x < graph_max_size; x++) {
                                        int y = graphic[previous_index].pixel_height + (int)(slope * (x - previous_index));
                                        if (y > GRAPH_DISPLAY_START_Y) {
                                            graphic[x].pixel_height = y;
                                            graphic[x].is_estimated = true;
                                        }
                                        else
                                            break;
                                    }
                                }
                                else {
                                    estimation_line_size = 0;
                                    estimated_playtime = 0;
                                }
                            }
                        }

                        is_estimation_computed = true;
                    }

                    total_duration += duration;
                    if (duration_to_pixel(total_duration) > graph_max_size)
                        b_quit = true;

                    previous_index = current_index;
                }
            }
            sqlite3_finalize(stmt);
            close_battery_log_db();
        }
    }
}

void renderPage()
{
    char sub_title[30];
    SDL_BlitSurface(background, NULL, screen, NULL);

    switch (current_zoom) {
    case 0:
        sprintf(sub_title, "%s", "16 HOURS VIEW");
        segment_duration = 7200;
        SDL_BlitSurface(right_arrow, NULL, screen, &(SDL_Rect){RIGHT_ARROW_X, RIGHT_ARROW_Y, ARROW_LENGHT, ARROW_WIDTH});
        break;
    case 1:
        sprintf(sub_title, "%s", "8 HOURS VIEW");
        segment_duration = 3600;
        SDL_BlitSurface(right_arrow, NULL, screen, &(SDL_Rect){RIGHT_ARROW_X, RIGHT_ARROW_Y, ARROW_LENGHT, ARROW_WIDTH});
        SDL_BlitSurface(left_arrow, NULL, screen, &(SDL_Rect){LEFT_ARROW_X, LEFT_ARROW_Y, ARROW_LENGHT, ARROW_WIDTH});
        break;
    case 2:
        sprintf(sub_title, "%s", "4 HOURS VIEW");
        segment_duration = 1800;
        SDL_BlitSurface(left_arrow, NULL, screen, &(SDL_Rect){LEFT_ARROW_X, LEFT_ARROW_Y, ARROW_LENGHT, ARROW_WIDTH});
        break;
    default:
        sprintf(sub_title, "%s", "8 HOURS VIEW");
        segment_duration = 3600;
        SDL_BlitSurface(right_arrow, NULL, screen, &(SDL_Rect){RIGHT_ARROW_X, RIGHT_ARROW_Y, ARROW_LENGHT, ARROW_WIDTH});
        SDL_BlitSurface(left_arrow, NULL, screen, &(SDL_Rect){LEFT_ARROW_X, LEFT_ARROW_Y, ARROW_LENGHT, ARROW_WIDTH});
        break;
    }
    if (estimation_line_size == 0)
        current_index = graph_max_size - (GRAPH_DISPLAY_SIZE_X * (int)(segment_duration / 1800));
    else
        current_index = begining_session_index;

    current_index -= (int)(current_page * (GRAPH_DISPLAY_SIZE_X * (int)(segment_duration / 1800)) / GRAPH_PAGE_SCROLL_SMOOTHNESS);

    if (current_index < 0)
        current_index = 0;

    switch_zoom_profile(segment_duration);

    renderText(label[0], font_Arkhip, color_white, &(SDL_Rect){LABEL1_X, LABEL_Y, 32, 32});
    renderText(label[1], font_Arkhip, color_white, &(SDL_Rect){LABEL2_X, LABEL_Y, 32, 32});
    renderText(label[2], font_Arkhip, color_white, &(SDL_Rect){LABEL3_X, LABEL_Y, 32, 32});
    renderText(label[3], font_Arkhip, color_white, &(SDL_Rect){LABEL4_X, LABEL_Y, 32, 32});

    renderText(sub_title, font_Arkhip, color_pastel_blue, &(SDL_Rect){SUB_TITLE_X, SUB_TITLE_Y, 100, 50});

    renderText(session_duration, font_Arkhip, color_white, &(SDL_Rect){LABEL_SESSION_X, LABEL_SESSION_Y, LABEL_SIZE_X, LABEL_SIZE_Y});
    renderText(current_percentage, font_Arkhip, color_white, &(SDL_Rect){LABEL_CURRENT_X, LABEL_CURRENT_Y, LABEL_SIZE_X, LABEL_SIZE_Y});
    renderTextAlignRight(session_left, font_Arkhip, color_white, &(SDL_Rect){LABEL_LEFT_X, LABEL_LEFT_Y, LABEL_SIZE_X, LABEL_SIZE_Y});
    renderTextAlignRight(session_best, font_Arkhip, color_white, &(SDL_Rect){LABEL_BEST_X, LABEL_BEST_Y, LABEL_SIZE_X, LABEL_SIZE_Y});

    int half_line_width = (int)(GRAPH_LINE_WIDTH) / 2;

    Uint32 white_pixel_color = SDL_MapRGBA(screen->format, 255, 255, 255, 0);
    Uint32 red_pixel_color = SDL_MapRGBA(screen->format, 255, 170, 170, 0);
    Uint32 blue_pixel_color = SDL_MapRGBA(screen->format, 89, 167, 255, 0);
    Uint32 pixel_color = white_pixel_color;

    int x;
    int y;
    int x_end = 0;
    int y_end = 0;
    int index;

    int zoom_level = (int)segment_duration / 1800;

    if (SDL_LockSurface(screen) == 0) {
        for (int i = 0; i < graph_max_size - current_index; i += zoom_level) {

            x = GRAPH_DISPLAY_START_X + (int)(i / zoom_level);
            y = graphic[i + current_index].pixel_height;

            bool is_charging = graphic[i + current_index].is_charging;
            bool is_estimated = graphic[i + current_index].is_estimated;
            //if ((!is_charging)
            if ((!is_charging) && (!is_estimated))
                pixel_color = white_pixel_color;
            else if (is_charging)
                pixel_color = red_pixel_color;
            else if (is_estimated) {
                pixel_color = blue_pixel_color;
                if ((y < GRAPH_DISPLAY_START_Y + 5) && (x < GRAPH_DISPLAY_SIZE_X + GRAPH_DISPLAY_START_X)) {
                    x_end = x - 12;
                    y_end = GRAPH_DISPLAY_SIZE_Y + GRAPH_DISPLAY_START_Y - 45;
                }
            }

            if (x < (GRAPH_DISPLAY_START_X + GRAPH_DISPLAY_SIZE_X)) {
                if (half_line_width >= 0) {
                    for (int k = -half_line_width; k <= half_line_width; k++) {
                        index = (480 - y + k) * screen->pitch + x * screen->format->BytesPerPixel;
                        *((Uint32 *)((Uint8 *)screen->pixels + index)) = pixel_color;
                    }
                }

                // Graph background
                if ((x % GRAPH_BACKGROUND_OPACITY) == 0) {
                    for (int k = y; k > GRAPH_DISPLAY_START_Y; k--) {
                        if ((k % GRAPH_BACKGROUND_OPACITY) == 0) {
                            index = (480 - k) * screen->pitch + x * screen->format->BytesPerPixel;
                            *((Uint32 *)((Uint8 *)screen->pixels + index)) = pixel_color;
                        }
                    }
                }
            }
        }
        SDL_UnlockSurface(screen);
        if (x_end != 0)
            SDL_BlitSurface(end_graph, NULL, screen, &(SDL_Rect){x_end, y_end, 24, 45});
    }

    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);
}

int main(int argc, char *argv[])
{
    init();
    KeyState keystate[320] = {(KeyState)0};
    render_waiting_screen();

    getDeviceModel();
    getDeviceSerial();
    compute_graph();
    renderPage();

    bool changed;

    while (!quit) {

        changed = false;
        if (updateKeystate(keystate, &quit, true, NULL)) {
            if (keystate[SW_BTN_B] == PRESSED)
                quit = true;
            if (keystate[SW_BTN_RIGHT] >= PRESSED) {
                if (current_page > 0)
                    current_page--;
                changed = true;
            }
            if (keystate[SW_BTN_LEFT] >= PRESSED) {
                int page_max = (int)(((GRAPH_MAX_FULL_PAGES) * (GRAPH_PAGE_SCROLL_SMOOTHNESS)) / (segment_duration / 1800));
                page_max -= GRAPH_PAGE_SCROLL_SMOOTHNESS;

                if (page_max > current_page)
                    current_page++;
                changed = true;
            }
            if ((keystate[SW_BTN_R1] >= PRESSED) || (keystate[SW_BTN_R2] >= PRESSED)) {
                if (current_zoom < 2) {
                    current_page = 0;
                    current_zoom++;
                    changed = true;
                }
            }
            if ((keystate[SW_BTN_L1] >= PRESSED) || (keystate[SW_BTN_L2] >= PRESSED)) {
                if (current_zoom > 0) {
                    current_page = 0;
                    current_zoom--;
                    changed = true;
                }
            }
        }

        if (!changed)
            continue;

        renderPage();
    }
    free_resources();
    return EXIT_SUCCESS;
}
