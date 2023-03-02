#include "./str.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

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

// Stores the trimmed input string into the given output buffer, which must be
// large enough to store the result.  If it is too small, the output is
// truncated.
size_t str_trim(char *out, size_t len, const char *str, bool first)
{
    if(len == 0)
        return 0;

    const char *end;
    size_t out_size;
    bool is_string = false;

    // Trim leading space
    while(strchr("\r\n\t {},", (unsigned char)*str) != NULL) str++;
    
    end = str + 1;
    
    if ((unsigned char)*str == '"') {
        is_string = true; str++;
        while(strchr("\r\n\"", (unsigned char)*end) == NULL) end++;
    }

    if(*str == 0)  // All spaces?
    {
        *out = 0;
        return 1;
    }

    // Trim trailing space
    if (first)
        while(strchr("\r\n\t {},", (unsigned char)*end) == NULL) end++;
    else {
        end = str + strlen(str) - 1;
        while(end > str && strchr("\r\n\t {},", (unsigned char)*end) != NULL) end--;
        end++;
    }

    if (is_string && (unsigned char)*(end-1) == '"')
        end--;

    // Set output size to minimum of trimmed string length and buffer size minus 1
    out_size = (end - str) < len-1 ? (end - str) : len-1;

    // Copy trimmed string and add null terminator
    memcpy(out, str, out_size);
    out[out_size] = 0;

    return out_size;
}

int str_endsWith(const char *str, const char *suffix)
{
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}
