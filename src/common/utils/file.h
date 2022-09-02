#ifndef UTILS_FILE_H__
#define UTILS_FILE_H__

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#define file_get(fp, path, format, dest) { if ((fp = fopen(path, "r"))) { fscanf(fp, format, dest); fclose(fp); } }
#define file_put(fp, path, format, value) { if ((fp = fopen(path, "w+"))) { fprintf(fp, format, value); fclose(fp); } }
#define file_put_sync(fp, path, format, value) { if ((fp = fopen(path, "w+"))) { fprintf(fp, format, value); fflush(fp); fsync(fileno(fp)); fclose(fp); } }

bool exists(const char *file_path);
bool is_file(const char *file_path);
bool is_dir(const char *file_path);
bool file_isModified(const char *path, time_t* old_mtime);

/**
 * @brief Create directories in dir_path using `mkdir -p` command.
 * 
 * @param dir_path The full directory path.
 * @return true If the path didn't exist (dirs were created).
 * @return false If the path exists (no dirs were created).
 */
bool mkdirs(const char *dir_path);

void file_readLastLine(const char* filename, char* out_str);

const char* file_read(const char *path);

bool file_write(const char *path, const char *str, uint32_t len);

void file_copy(const char *src_path, const char *dest_path);

char *file_removeExtension(char* myStr);

const char *file_getExtension(const char *filename);

char* file_parseKeyValue(const char *file_path, const char *key_in, char *value_out, char divider, int select_index);

void file_changeKeyValue(const char *file_path, const char *key, const char *replacement_line);

#endif // UTILS_FILE_H__
