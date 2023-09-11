#ifndef BATTERY_MONITOR_UI_H
#define BATTERY_MONITOR_UI_H
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <math.h>
#include <signal.h>
#include <sqlite3/sqlite3.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "system/display.h"
#include "system/keymap_sw.h"
#include "system/system.h"
#include "utils/file.h"
#include "utils/keystate.h"
#include "utils/log.h"
#include "utils/str.h"
#include "utils/surfaceSetAlpha.h"

#define GRAPH_LINE_WIDTH 1

// Space between pixels, higher is more transparent
#define GRAPH_BACKGROUND_OPACITY 4

// A multiple of 4 is recommended
#define GRAPH_MAX_FULL_PAGES 8

#define GRAPH_DISPLAY_SIZE_X 583
#define GRAPH_DISPLAY_SIZE_Y 324
#define GRAPH_DISPLAY_START_X 29
#define GRAPH_DISPLAY_START_Y 79
#define GRAPH_DISPLAY_DURATION 16200
#define GRAPH_PAGE_SCROLL_SMOOTHNESS 12
#define GRAPH_MIN_SESSION_FOR_ESTIMATION 1200
#define GRAPH_MAX_PLAUSIBLE_ESTIMATION 54000
#define GRAPH_ESTIMATED_LINE_GAP 20

#define LABEL_Y 410

#define LABEL1_X 150
#define LABEL2_X 278
#define LABEL3_X 407
#define LABEL4_X 538

#define SUB_TITLE_X 255
#define SUB_TITLE_Y 32

#define LABEL_SESSION_X 100
#define LABEL_SESSION_Y 439
#define LABEL_CURRENT_X 108
#define LABEL_CURRENT_Y 454
#define LABEL_LEFT_X 630
#define LABEL_LEFT_Y 438
#define LABEL_BEST_X 630
#define LABEL_BEST_Y 453

#define LABEL_SIZE_X 65
#define LABEL_SIZE_Y 15

#define RIGHT_ARROW_X 591
#define RIGHT_ARROW_Y 13

#define LEFT_ARROW_X 0
#define LEFT_ARROW_Y 13

#define ARROW_LENGHT 48
#define ARROW_WIDTH 28

#endif //BATTERY_MONITOR_UI_H
