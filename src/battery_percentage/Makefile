# CROSS_COMPILE=/usr/bin/arm-linux-gnueabihf-

TARGET = mainUiBatPerc

CC 		= $(CROSS_COMPILE)gcc
CXX 	= $(CROSS_COMPILE)g++
STRIP 	= $(CROSS_COMPILE)strip

SOURCES = . ../../include/cJSON
CFILES = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.c))
CPPFILES = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.cpp))
OFILES = $(CFILES:.c=.o) $(CPPFILES:.cpp=.o)

# NOTE: These flags aren't strickly necessary...
CFLAGS = -I../../include -I../common -marm -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -march=armv7ve 
CXXFLAGS = $(CFLAGS)
LDFLAGS = -L../static/.tmp_update/.data/Core/miyoo/lib -lSDL -lSDL_ttf -lSDL_image -pthread -lshmvar -lpng


$(TARGET): $(OFILES)
	@$(CXX) $(OFILES) -o $@ $(LDFLAGS)
# $(STRIP) $@

%.o: %.c
	@echo Building $@
	@$(CC) $(CFLAGS) -c $< -o $@

# %.o: %.cpp
# 	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -f $(TARGET) $(OFILES)
	@echo Cleaned

install:
	@echo "do nothing for install"
