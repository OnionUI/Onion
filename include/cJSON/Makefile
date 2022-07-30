TARGET = ../../lib/libcjson.so

CC = $(CROSS_COMPILE)g++

$(TARGET):
	$(CC) -shared -o $(TARGET) -fPIC cJSON.c

clean:
	rm -f $(TARGET)
