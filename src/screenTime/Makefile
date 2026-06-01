INCLUDE_UTILS = 0
CFILES := ../common/utils/retroarch_cmd.c ../common/utils/udp.c
include ../common/config.mk

TARGET = screenTime
CFLAGS := $(CFLAGS) -D_DEFAULT_SOURCE
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
SQLITE_LDFLAGS := $(shell xcrun --show-sdk-path)/usr/lib/libsqlite3.tbd
else
SQLITE_LDFLAGS := -lsqlite3
endif
LDFLAGS := $(LDFLAGS) $(SQLITE_LDFLAGS)

include ../common/commands.mk
include ../common/recipes.mk
