INCLUDE_CJSON=1
OFILES := $(OFILES) ../common/utils/keystate.o ../common/utils/msleep.o
include ../common/config.mk

TARGET = gameSwitcher
LDFLAGS := $(LDFLAGS) -lSDL -lSDL_ttf -lSDL_image -lpng

include ../common/commands.mk
include ../common/recipes.mk
