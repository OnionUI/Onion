ifeq (,$(BUILD_DIR))
BUILD_DIR=.
endif

TARGET = $(BUILD_DIR)/lastGame

CC 		= $(CROSS_COMPILE)gcc
CXX 	= $(CROSS_COMPILE)g++
STRIP 	= $(CROSS_COMPILE)strip

SOURCES = . ../../include/cjson
CFILES = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.c))
CPPFILES = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.cpp))
OFILES = $(CFILES:.c=.o) $(CPPFILES:.cpp=.o)

# NOTE: These flags aren't strickly necessary...
CFLAGS = -I../../include -marm -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -march=armv7ve 
CXXFLAGS = $(CFLAGS)
LDFLAGS = -lSDL -lSDL_image

$(TARGET): $(OFILES)
	$(CXX) $(OFILES) -o $@ $(LDFLAGS)
	#$(STRIP) $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# %.o: %.cpp
# 	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OFILES)


install:
	echo do nothing for install
