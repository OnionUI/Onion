INCLUDE_UTILS = 0
include ../common/config.mk

TARGET = libmainuihooks.so
CFLAGS := $(CFLAGS) -fpic
LDFLAGS := $(LDFLAGS) -shared -ldl -lSDL -lSDL_ttf

include ../common/commands.mk
include ../common/recipes.mk
