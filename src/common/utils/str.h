#ifndef UTILS_STR_H__
#define UTILS_STR_H__

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define STR_MAX 256
#define concat(ptr, str1, str2) { strcpy(ptr, str1); strcat(ptr, str2); }

bool str_getLastNumber(char* str, long* out_val);
char* str_split(char *str, const char *delim);
char* str_replace(char *orig, char *rep, char *with);

// Stores the trimmed input string into the given output buffer, which must be
// large enough to store the result.  If it is too small, the output is
// truncated.
size_t str_trim(char *out, size_t len, const char *str, bool first);

int str_endsWith(const char *str, const char *suffix);

#endif // UTILS_STR_H__
