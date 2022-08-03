ifeq (,$(BUILD_DIR))
BUILD_DIR=.
endif

TARGET = $(BUILD_DIR)/installUI

CC 	= $(CROSS_COMPILE)gcc

SOURCES = . ../../include/cjson
CFILES = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.c))
OFILES = $(CFILES:.c=.o)

CFLAGS = -I../../include -O3 -Wall
LDFLAGS = -s -lSDL -lSDL_ttf -lSDL_image

$(TARGET): $(OFILES)
	$(CC) $(OFILES) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OFILES)
