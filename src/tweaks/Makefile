INCLUDE_SHMVAR=1
INCLUDE_CJSON=1
include ../common/config.mk

TARGET = tweaks
CFLAGS := $(CFLAGS) -D_DEFAULT_SOURCE
LDFLAGS := $(LDFLAGS) -lSDL -lSDL_ttf -lSDL_image -lSDL_mixer -lSDL_rotozoom -pthread -lkbinput

include ../common/commands.mk
include ../common/recipes.mk
