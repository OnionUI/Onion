TEST = 1
INCLUDE_UTILS = 0
CFILES := ../../src/screenTime/screenTime.c
include ../../src/common/config.mk

TARGET = test_screenTime
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
SQLITE_LDFLAGS := $(shell xcrun --show-sdk-path)/usr/lib/libsqlite3.tbd
else
SQLITE_LDFLAGS := -lsqlite3
endif
LDFLAGS := $(LDFLAGS) $(SQLITE_LDFLAGS) -lgtest -lgtest_main -lpthread

include ../../src/common/commands.mk
include ../../src/common/recipes.mk
