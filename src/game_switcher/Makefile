TARGET = onionLauncher

CC 	= $(CROSS_COMPILE)g++

SOURCES = . ../../include/cJSON ../../include/SDL ../../include/png ../common/gfx
CFILES = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.c))
OFILES = $(CFILES:.c=.o)

CFLAGS = -O3 -marm -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -march=armv7ve -Wall
LDFLAGS = -lSDL -lSDL_ttf -lmi_sys -lmi_gfx -s -lSDL -lSDL_image -lpng

$(TARGET): $(OFILES)
	$(CC) $(OFILES) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@




clean:
	rm -f $(TARGET) $(OFILES)
