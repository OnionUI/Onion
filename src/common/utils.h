#ifndef UTILS_H__
#define UTILS_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

#define MAX_LEN 256

bool file_exists(const char *filename) {
	struct stat buffer;
	return (stat(filename, &buffer) == 0);
}

bool file_isModified(const char *path, time_t* old_mtime) {
    struct stat file_stat;
    int err = stat(path, &file_stat);
    if (err != 0) {
        perror(" [file_isModified] stat");
        exit(errno);
    }
    if (file_stat.st_mtime > *old_mtime) {
		*old_mtime = file_stat.st_mtime;
		return true;
	}
	return false;
}

void file_readLastLine(const char* filename, char* out_str)
{
	FILE *fd;
	int size;
	char buff[MAX_LEN + 1];
	char* token = NULL;

	if ((fd = fopen(filename, "rb")) != NULL) {
		// get file size
		fseek(fd, 0L, SEEK_END);
		size = ftell(fd);
		fseek(fd, 0L, SEEK_SET);

		int max_len = size < MAX_LEN ? size + 1 : MAX_LEN;
		if (max_len <= 1)
			return;

		// get the last line
		fseek(fd, -max_len, SEEK_END);
		fread(buff, max_len-1, 1, fd);

		// cleanup
		fclose(fd);
		buff[max_len-1] = '\0';

		token = strtok(buff, "\n");
		while (token != NULL) {
			if (strlen(token) > 0)
				snprintf(out_str, MAX_LEN, "%s", token);
			token = strtok(NULL, "\n");
		}
	}
}

const char* file_readAll(const char *path)
{
    FILE *f = NULL;
    char *buffer = NULL;
    long length = 0;

	if (!file_exists(path))
		return NULL;

    if ((f = fopen(path, "rb"))) {
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);
        buffer = (char *)malloc((length + 1) * sizeof(char));
        if (buffer)
        fread(buffer, sizeof(char), length, f);
        fclose(f);
    }
    buffer[length] = '\0';

    return buffer;
}

bool str_getLastNumber(char* str, long* out_val)
{
	char *p = str;
	long val = -1;

    while (*p) {
        if (isdigit(*p))
            val = strtol(p, &p, 10);
		else p++;
    }

	if (val != -1)
		*out_val = val;

	return val != -1;
}

#endif // UTILS_H__
