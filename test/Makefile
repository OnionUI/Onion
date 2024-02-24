TEST = 1
INCLUDE_UTILS = 0
CFILES := ../src/infoPanel/imagesCache.c
include ../src/common/config.mk

TARGET = test
LDFLAGS := $(LDFLAGS) -L../lib -s -lSDL_image -lSDL -lSDL_rotozoom -lgtest -lgtest_main -lpthread

include ../src/common/commands.mk
include ../src/common/recipes.mk