INCLUDE_SHMVAR=1
INCLUDE_CJSON=1
include ../common/config.mk

CFILES := $(CFILES) \
	../common/utils/udp.c

TARGET = keymon
CFLAGS := $(CFLAGS) -Os -ffunction-sections -fdata-sections
LDFLAGS := $(LDFLAGS) -lpthread -lpng -Wl,--gc-sections

include ../common/commands.mk
include ../common/recipes.mk
