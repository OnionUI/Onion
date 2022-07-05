

TARGET = keymon
TARGET2 = keymon_onion

CC 	= $(CROSS_COMPILE)gcc

SOURCES = . 
CFILES = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.c))
OFILES = $(CFILES:.c=.o)

CFLAGS = -Os -marm -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -march=armv7ve+simd
CFLAGS += -ffunction-sections -fdata-sections -Wall
LDFLAGS = -lshmvar -lpthread -lpng -Wl,--gc-sections -s

$(TARGET): $(OFILES)
	$(CC) $(OFILES) -o $@ $(LDFLAGS)
	cp -f $(TARGET) $(TARGET2)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(TARGET2) $(OFILES)
