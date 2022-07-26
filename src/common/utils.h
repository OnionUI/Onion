#ifndef UTILS_H__
#define UTILS_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

bool file_exists(const char *filename) {
	struct stat buffer;
	return (stat(filename, &buffer) == 0);
}

const char* loadFile(const char *path)
{
	char* buffer = NULL;

    if (!file_exists(path))
        return buffer;

	long length = 0;
	FILE * f = fopen(path, "rb");

	if (f) {
		fseek(f, 0, SEEK_END);
		length = ftell(f);
		fseek(f, 0, SEEK_SET);
		buffer = (char*)malloc((length+1)*sizeof(char));
		if (buffer)
			fread(buffer, sizeof(char), length, f);
		fclose(f);
	}
	buffer[length] = '\0';

	return buffer; 
}

#endif // UTILS_H__
