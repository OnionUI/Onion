INCLUDE_CJSON=1
CFILES := \
	../common/utils/imageCache.c
include ../common/config.mk

TARGET = installUI
LDFLAGS := $(LDFLAGS) -lSDL -lSDL_ttf -lSDL_image -pthread

include ../common/commands.mk
include ../common/recipes.mk
