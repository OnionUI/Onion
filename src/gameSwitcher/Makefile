INCLUDE_CJSON=1
CFILES := \
	../common/utils/imageCache.c
include ../common/config.mk

TARGET = gameSwitcher
LDFLAGS := $(LDFLAGS) -lSDL -lSDL_ttf -lSDL_image -lSDL_mixer -lSDL_rotozoom -lpng -pthread

include ../common/commands.mk
include ../common/recipes.mk
