INCLUDE_SHMVAR=1
INCLUDE_CJSON=1
CFILES := ../screenTime/screenTime.c
include ../common/config.mk

TARGET = tweaks
CFLAGS := $(CFLAGS) -D_DEFAULT_SOURCE -DHAS_AUDIO
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
SQLITE_LDFLAGS := $(shell xcrun --show-sdk-path)/usr/lib/libsqlite3.tbd
else
SQLITE_LDFLAGS := -lsqlite3
endif
LDFLAGS := $(LDFLAGS) $(SQLITE_LDFLAGS) -lSDL -lSDL_ttf -lSDL_image -lSDL_mixer -lSDL_rotozoom -pthread -lkbinput

include ../common/commands.mk
include ../common/recipes.mk

./tweaks.o: $(wildcard ./*.h) ../screenTime/screenTime.h
../screenTime/screenTime.o: ../screenTime/screenTime.h
