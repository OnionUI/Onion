CROSS_COMPILE ?= /opt/miyoomini/bin/arm-linux-gnueabihf-
ARCH = -marm -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -march=armv7ve+simd

TARGET = jpg2png

CC 	= $(CROSS_COMPILE)gcc

SOURCES = . 
CFILES = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.c))
OFILES = $(CFILES:.c=.o)

CFLAGS = -Os $(ARCH) -ffunction-sections -fdata-sections -Wall
LDFLAGS = $(ARCH) -lmi_sys -lmi_gfx -lpng -Wl,-Bstatic -ljpeg -Wl,-Bdynamic

$(TARGET): $(OFILES)
	$(CC) $(OFILES) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OFILES)

install:
	echo do nothing for install
