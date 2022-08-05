include ../common/common.mk

TARGET := $(BUILD_DIR)/packageManager
LDFLAGS := $(LDFLAGS) -s -lSDL -lSDL_image -lSDL_ttf

$(TARGET): $(OFILES)
	$(CC) $(OFILES) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OFILES)
