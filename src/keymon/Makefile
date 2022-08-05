INCLUDE_SHMVAR=1
INCLUDE_CJSON=1
include ../common/common.mk

TARGET := $(BUILD_DIR)/keymon
CFLAGS := $(CFLAGS) -Os -ffunction-sections -fdata-sections
LDFLAGS := $(LDFLAGS) -lpthread -lpng -Wl,--gc-sections -s

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
