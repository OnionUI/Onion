include ../common/common.mk

TARGET := $(BUILD_DIR)/chargingState
LDFLAGS := $(LDFLAGS) -s -lSDL -lSDL_image

$(TARGET): $(OFILES)
	$(CC) $(OFILES) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OFILES)
