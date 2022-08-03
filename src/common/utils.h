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
#include <SDL/SDL.h>

#define MAX_LEN 256
#define concat(ptr,str1,str2) { strcpy(ptr, str1); strcat(ptr, str2); }

bool file_exists(const char *filename) {
	struct stat buffer;
	return (stat(filename, &buffer) == 0);
}

bool dotfile(const char *key) {
    char filename[MAX_LEN];
    concat(filename, "/mnt/SDCARD/.tmp_update/.", key);
    return file_exists(filename);
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

SDL_Color hex2sdl(char *input) {
	char *ptr;
    if (input[0] == '#')
        input++;
    unsigned long value = strtoul(input, &ptr, 16);
    SDL_Color color = {
    	(value >> 16) & 0xff,
    	(value >> 8) & 0xff,
    	(value >> 0) & 0xff
	};
    return color;
}

Uint32 colorToUint(SDL_Color color)
{
	return (Uint32)((color.r << 16) + (color.g << 8) + (color.b << 0));
}

SDL_Color uintToColor(Uint32 color)
{
	SDL_Color sdl_color;
	sdl_color.unused = 255;
	sdl_color.r = (color >> 16) & 0xFF;
	sdl_color.g = (color >> 8) & 0xFF;
	sdl_color.b = color & 0xFF;
	return sdl_color;
}

char* str_split(char *str, const char *delim)
{
    char *p = strstr(str, delim);
    if (p == NULL) return NULL;     // delimiter not found
    *p = '\0';                      // terminate string after head
    return p + strlen(delim);       // return tail substring
}

char* str_replace(char *orig, char *rep, char *with) {
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep (the string to remove)
    int len_with; // length of with (the string to replace rep with)
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    // sanity checks and initialization
    if (!orig || !rep)
        return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL; // empty rep causes infinite loop during count
    if (!with)
        with = "";
    len_with = strlen(with);

    // count the number of replacements needed
    ins = orig;
    for (count = 0; (tmp = strstr(ins, rep)); ++count)
        ins = tmp + len_rep;

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}


#endif // UTILS_H__
