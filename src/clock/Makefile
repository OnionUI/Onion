SOURCES := $(SOURCES) ./font

include ../common/config.mk

TARGET = clock
LDFLAGS := $(LDFLAGS) -lSDL -lpthread -lmi_sys -lmi_gfx
CFLAGS := $(CFLAGS) -I./font -Os -flto -fno-fat-lto-objects

include ../common/commands.mk
include ../common/recipes.mk
