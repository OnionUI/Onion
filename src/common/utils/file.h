#ifndef UTILS_FILE_H__
#define UTILS_FILE_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#include "log.h"
#include "msleep.h"

#define file_get(fp, path, format, dest) { if ((fp = fopen(path, "r"))) { fscanf(fp, format, dest); fclose(fp); } }
#define file_put(fp, path, format, value) { if ((fp = fopen(path, "w+"))) { fprintf(fp, format, value); fclose(fp); } }
#define file_put_sync(fp, path, format, value) { if ((fp = fopen(path, "w+"))) { fprintf(fp, format, value); fflush(fp); fsync(fileno(fp)); fclose(fp); } }

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
	char buff[256];
	char* token = NULL;

	if ((fd = fopen(filename, "rb")) != NULL) {
		// get file size
		fseek(fd, 0L, SEEK_END);
		size = ftell(fd);
		fseek(fd, 0L, SEEK_SET);

		int max_len = size < 255 ? size + 1 : 255;
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
				snprintf(out_str, 255, "%s", token);
			token = strtok(NULL, "\n");
		}
	}
}

const char* file_read(const char *path)
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

bool file_write(const char *path, const char *str, uint32_t len)
{
    uint32_t fd;
    if ((fd = open(path, O_WRONLY)) == 0)
        return false;
    if (write(fd, str, len) == -1)
        return false;
    close(fd);
    return true;
}

char *file_removeExtension(char* myStr)
{
    if (myStr == NULL) return NULL;
    char *retStr = (char*)malloc(strlen(myStr) + 1);
    char *lastExt;
    if (retStr == NULL) return NULL;
    strcpy(retStr, myStr);
    if ((lastExt = strrchr(retStr, '.')) != NULL)
        *lastExt = '\0';
    return retStr;
}

#endif // UTILS_FILE_H__
