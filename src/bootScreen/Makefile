INCLUDE_CJSON=1
INCLUDE_SHMVAR=1
OFILES := $(OFILES) ../common/utils/msleep.o
include ../common/config.mk

TARGET = bootScreen
LDFLAGS := $(LDFLAGS) -s -lSDL -lSDL_image -lSDL_ttf -lSDL_mixer

include ../common/commands.mk
include ../common/recipes.mk
