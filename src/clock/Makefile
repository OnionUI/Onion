# PRGNAME     = clock
# CC			= /opt/miyoomini83/bin/arm-linux-gnueabihf-gcc

# OBJS		= main.o gfx.o font/font_drawing.o

# CFLAGS	=  -Os -marm -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -march=armv7ve+simd
# CFLAGS	+= -ffunction-sections -fdata-sections -Wall
# CFLAGS	+= -std=gnu99 -I. -Ifont -flto
# LDFLAGS	= -lc -lgcc -lm -lSDL -lpthread -lmi_sys -lmi_gfx
# LDFLAGS += -Wl,--gc-sections -s

# # Rules to make executable
# $(PRGNAME): $(OBJS)  
# 	$(CC) $(CFLAGS)  -o $(PRGNAME) $^ $(LDFLAGS)

# $(OBJ_C) : %.o : %.c
# 	$(CC) $(CFLAGS) -std=gnu99 -c -o $@ $<

# clean:
# 	rm -f $(PRGNAME) *.o font/*.o

SOURCES := $(SOURCES) ./font

include ../common/config.mk

TARGET = clock
LDFLAGS := $(LDFLAGS) -lSDL -lSDL_ttf -lSDL_image -lSDL_rotozoom -lpthread -lmi_sys -lmi_gfx
CFLAGS := $(CFLAGS) -I./font

include ../common/commands.mk
include ../common/recipes.mk
