TARGET = ../../lib/libgfx.so

CC = $(CROSS_COMPILE)g++

$(TARGET):
	$(CC) -shared -o $(TARGET) -fPIC gfx.c

clean:
	rm -f $(TARGET)
