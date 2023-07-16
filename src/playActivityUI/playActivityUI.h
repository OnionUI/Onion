#ifndef PLAY_ACTIVITY_UI_H
#define PLAY_ACTIVITY_UI_H

#include "sys/ioctl.h"
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "system/keymap_sw.h"
#include "system/system.h"
#include "utils/config.h"
#include "utils/file.h"
#include "utils/keystate.h"
#include "utils/log.h"
#include "utils/str.h"

#include "../playActivity/playActivityDB.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

#define IMG_MAX_WIDTH 80
#define IMG_MAX_HEIGHT 80

#endif
