#ifndef UTILS_H__
#define UTILS_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <SDL/SDL.h>

#define MAX_LEN 256
#define concat(ptr,str1,str2) { strcpy(ptr, str1); strcat(ptr, str2); }

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
    char *retStr;
    char *lastExt;
    if (myStr == NULL) return NULL;
    if ((retStr = malloc(strlen(myStr) + 1)) == NULL)
		return NULL;
    strcpy(retStr, myStr);
    if ((lastExt = strrchr(retStr, '.')) != NULL)
        *lastExt = '\0';
    return retStr;
}

int getBatteryPercentage()
{
    int percentage = 0;

    if (file_exists("/tmp/percBat")) {
        char val[5];
        const char *cPercBat = file_readAll("/tmp/percBat");
        strcpy(val, cPercBat);
        percentage = atoi(val);
    }

    return percentage;
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

//
//	Trim Strings for reading json (EGGS)
//
char* trimstr(char* str, uint32_t first) {
	char *firstchar, *firstlastchar, *lastfirstchar, *lastchar;
	uint32_t i;

	firstchar = firstlastchar = lastfirstchar = lastchar = 0;

	for (i=0; i<strlen(str); i++) {
		if ((str[i]!='\r')&&(str[i]!='\n')&&(str[i]!=' ')&&(str[i]!='\t')&&
		    (str[i]!='{')&&(str[i]!='}')&&(str[i]!=',')) {
			if (!firstchar) {
				firstchar = &str[i];
				lastfirstchar = &str[i];
			}
			if (i) {
				if ((str[i-1]=='\r')||(str[i-1]=='\n')||(str[i-1]==' ')||(str[i-1]=='\t')||
				    (str[i-1]=='{')||(str[i-1]=='}')||(str[i-1]==',')) {
					lastfirstchar = &str[i];
				}
			}
			if (str[i] == '"') {
				for (i++; i<(strlen(str)-1); i++) {
					if ((str[i]=='\r')||(str[i]=='\n')||(str[i]=='"')) break;
				}
			}
			lastchar = &str[i];
		} else {
			if (!firstlastchar) {
				firstlastchar = lastchar;
			}
		}
	}
	if (first) {
		lastfirstchar = firstchar;
		lastchar = firstlastchar;
	}
	if (lastchar) {
		lastchar[1] = 0;
	}
	if (lastfirstchar) return lastfirstchar;
	return 0;
}


#endif // UTILS_H__
