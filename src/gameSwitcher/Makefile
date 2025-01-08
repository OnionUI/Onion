INCLUDE_CJSON=1
include ../common/config.mk

CFILES := $(CFILES) \
	../common/utils/udp.c \
	../common/utils/retroarch_cmd.c

TARGET = gameSwitcher
LDFLAGS := $(LDFLAGS) -lSDL -lSDL_ttf -lSDL_image -lSDL_rotozoom -lpng -lsqlite3 -pthread

include ../common/commands.mk
include ../common/recipes.mk
