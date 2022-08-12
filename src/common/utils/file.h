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

bool exists(const char *file_path) {
	struct stat buffer;
	return stat(file_path, &buffer) == 0;
}

bool is_file(const char *file_path) {
    struct stat buffer;
    return stat(file_path, &buffer) == 0 && S_ISREG(buffer.st_mode);
}

bool is_dir(const char *file_path) {
    struct stat buffer;
    return stat(file_path, &buffer) == 0 && S_ISDIR(buffer.st_mode);
}

bool file_isModified(const char *path, time_t* old_mtime) {
    struct stat file_stat;
    if (stat(path, &file_stat) == 0 && file_stat.st_mtime > *old_mtime) {
		*old_mtime = file_stat.st_mtime;
		return true;
	}
	return false;
}

/**
 * @brief Create directories in dir_path using `mkdir -p` command.
 * 
 * @param dir_path The full directory path.
 * @return true If the path didn't exist (dirs were created).
 * @return false If the path exists (no dirs were created).
 */
bool mkdirs(const char *dir_path)
{
    if (!exists(dir_path)) {
        char dir_cmd[512];
        sprintf(dir_cmd, "mkdir -p \"%s\"", dir_path);
        system(dir_cmd);
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

	if (!exists(path))
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

void file_copy(const char *src_path, const char *dest_path)
{
    char system_cmd[512];
    snprintf(system_cmd, 511, "cp -f \"%s\" \"%s\"", src_path, dest_path);
    system(system_cmd);
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

const char *file_getExtension(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return dot + 1;
}

char* file_parseKeyValue(const char *file_path, const char *key_in, char *value_out, char divider) {
    FILE *fp;
    int f;
    char key[256], val[256];
    char key_search[STR_MAX];
    char search_str[STR_MAX];
    sprintf(search_str, "%%255[^%c]%c%%255[^\n]\n", divider, divider);

    *value_out = 0;
    if ( (fp = fopen(file_path, "r")) ) {
        key[0] = 0; val[0] = 0;
        while ((f = fscanf(fp, search_str, key, val)) != EOF) {
            if (!f) { if (fscanf(fp, "%*[^\n]\n") == EOF) break; else continue; }
            if (str_trim(key_search, 256, key, true)) {
                if (strcmp(key_search, key_in) == 0) {
                    str_trim(value_out, 256, val, false);
                    break;
                }
            }
            key[0] = 0; val[0] = 0;
        }
        fclose(fp);
    }

    if (*value_out == 0) return NULL;
    return value_out;
}

#endif // UTILS_FILE_H__
