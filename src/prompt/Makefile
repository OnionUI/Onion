INCLUDE_SHMVAR=1
INCLUDE_CJSON=1
include ../common/common.mk

TARGET := $(BUILD_DIR)/prompt
LDFLAGS := $(LDFLAGS) -lSDL -lSDL_ttf -lSDL_image -lSDL_rotozoom -pthread

$(TARGET): $(OFILES)
	@$(CXX) $(OFILES) -o $@ $(LDFLAGS)
	@$(STRIP) $@

%.o: %.c
	@echo gcc $@
	@$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	@echo g++ $@
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -f $(TARGET) $(OFILES)
	@echo Cleaned

install:
	@echo "do nothing for install"
