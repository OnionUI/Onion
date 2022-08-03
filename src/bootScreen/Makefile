INCLUDE_SHMVAR=1
include ../common/common.mk

TARGET := $(BUILD_DIR)/bootScreen
LDFLAGS := $(LDFLAGS) -s -lSDL -lSDL_image -lSDL_ttf -lmi_sys -lmi_gfx

$(TARGET): $(OFILES)
	$(CC) $(OFILES) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OFILES)
