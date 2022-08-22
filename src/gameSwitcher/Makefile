INCLUDE_CJSON=1
INCLUDE_UTILS=1
CFILES := \
	../common/utils/imageCache.c
include ../common/config.mk

TARGET = gameSwitcher
LDFLAGS := $(LDFLAGS) -lSDL -lSDL_ttf -lSDL_image -lpng -pthread

include ../common/commands.mk
include ../common/recipes.mk
