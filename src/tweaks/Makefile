INCLUDE_SHMVAR=1
INCLUDE_CJSON=1
OFILES := $(OFILES) ../common/utils/sdl_init.o ../common/utils/msleep.o ../common/utils/keystate.o
include ../common/config.mk

TARGET = tweaks
LDFLAGS := $(LDFLAGS) -lSDL -lSDL_ttf -lSDL_image -lSDL_mixer -lSDL_rotozoom

include ../common/commands.mk
include ../common/recipes.mk
