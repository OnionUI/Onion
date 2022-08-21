INCLUDE_SHMVAR=1
INCLUDE_CJSON=1
OFILES := $(OFILES) ../common/utils/msleep.o
include ../common/config.mk

TARGET = keymon
CFLAGS := $(CFLAGS) -Os -ffunction-sections -fdata-sections
LDFLAGS := $(LDFLAGS) -lpthread -lpng -Wl,--gc-sections -s

include ../common/commands.mk
include ../common/recipes.mk
