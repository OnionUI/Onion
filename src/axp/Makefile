include ../common/config.mk

TARGET = axp
CFLAGS := $(CFLAGS) -Os -ffunction-sections -fdata-sections -Wall
LDFLAGS := $(LDFLAGS) -Wl,--gc-sections -s

include ../common/commands.mk
include ../common/recipes.mk
