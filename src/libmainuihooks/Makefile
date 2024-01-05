INCLUDE_CJSON=1
INCLUDE_UTILS=0
include ../common/config.mk

TARGET = libmainuihooks.so
CFLAGS := $(CFLAGS) -fPIC
LDFLAGS := $(LDFLAGS) -shared -ldl -lpthread -lSDL_ttf -lSDL_rotozoom

include ../common/commands.mk
include ../common/recipes.mk