ifeq (,$(BUILD_DIR))
BUILD_DIR=.
endif

TARGET = $(BUILD_DIR)/prompt

CC 		= $(CROSS_COMPILE)gcc
CXX 	= $(CROSS_COMPILE)g++
STRIP 	= $(CROSS_COMPILE)strip

SOURCES = .
CFILES = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.c))
CPPFILES = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.cpp))
OFILES = $(CFILES:.c=.o) $(CPPFILES:.cpp=.o)

CFLAGS = -I../../include -I../common
CXXFLAGS = $(CFLAGS)
LDFLAGS = -L../../lib -lSDL -lSDL_ttf -lSDL_image -lSDL_rotozoom -lshmvar -lcjson


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
