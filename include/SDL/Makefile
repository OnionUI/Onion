.PHONY: clean

TARGET = ../../lib/libSDL_rotozoom.so

CC = $(CROSS_COMPILE)gcc

$(TARGET):
	$(CC) -shared -o $(TARGET) -fPIC SDL_rotozoom.c

clean:
	rm -f $(TARGET)
