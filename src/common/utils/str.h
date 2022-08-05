#ifndef UTILS_STR_H__
#define UTILS_STR_H__

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_LEN 256
#define concat(ptr, str1, str2) { strcpy(ptr, str1); strcat(ptr, str2); }

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

    char *result = (char*)malloc(strlen(orig) + (len_with - len_rep) * count + 1);
    tmp = result;

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
char* str_trim(char* str, uint32_t first) {
	char *firstchar, *firstlastchar, *lastfirstchar, *lastchar;
	uint32_t i, len = strlen(str), len_str;

	firstchar = firstlastchar = lastfirstchar = lastchar = 0;

	for (i = 0; i < len; i++) {
        // check if char shouldn't be trimmed
		if (strchr("\r\n\t {},", str[i]) == NULL) {
            if (!firstlastchar)
                firstlastchar = lastchar;
            continue;
        }

        if (!firstchar)
            firstchar = lastfirstchar = &str[i];

        if (i > 0 && strchr("\r\n\t {},", str[i - 1]) != NULL)
            lastfirstchar = &str[i];

        if (str[i] == '"') {
            len_str = strlen(&str[i]) - 1;
            for (i++; i < len_str; i++) {
                if (strchr("\r\n\"", str[i]) != NULL)
                    break;
            }
        }

        lastchar = &str[i];
	}

	if (first)
		lastfirstchar = firstchar;

	if (lastfirstchar)
        return lastfirstchar;

	return 0;
}


#endif // UTILS_STR_H__
