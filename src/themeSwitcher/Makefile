ifeq (,$(BUILD_DIR))
BUILD_DIR=.
endif

TARGET = $(BUILD_DIR)/themeSwitcher

CC 	= $(CROSS_COMPILE)g++

SOURCES = . ../../include/cjson
CFILES = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.c))
OFILES = $(CFILES:.c=.o)

CFLAGS = -I../../include -O3 -marm -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -march=armv7ve -Wall
LDFLAGS = -L../../lib -lSDL -lSDL_ttf -lmi_sys -lmi_gfx -s -lSDL -lSDL_image

$(TARGET): $(OFILES)
	$(CC) $(OFILES) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OFILES)
