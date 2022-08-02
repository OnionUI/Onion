ifeq (,$(BUILD_DIR))
BUILD_DIR=.
endif

TARGET = $(BUILD_DIR)/installPrompt

CC = $(CROSS_COMPILE)gcc

SOURCES = . 
CFILES = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.c))
OFILES = $(CFILES:.c=.o)

CFLAGS = -I../../include -I../common -O3 -Wall
LDFLAGS = -L../../lib -s -lSDL -lSDL_image -lSDL_ttf -lcjson -lSDL_rotozoom

$(TARGET): $(OFILES)
	$(CC) $(OFILES) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OFILES)
