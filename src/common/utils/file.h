#ifndef UTILS_FILE_H__
#define UTILS_FILE_H__

#include <dirent.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#ifndef DT_UNKNOWN
/** d_type value when the type is not known. */
#define DT_UNKNOWN 0
/** d_type value for a FIFO. */
#define DT_FIFO 1
/** d_type value for a character device. */
#define DT_CHR 2
/** d_type value for a directory. */
#define DT_DIR 4
/** d_type value for a block device. */
#define DT_BLK 6
/** d_type value for a regular file. */
#define DT_REG 8
/** d_type value for a symbolic link. */
#define DT_LNK 10
/** d_type value for a socket. */
#define DT_SOCK 12
#define DT_WHT 14
#endif

#define file_get(fp, path, format, dest) \
    {                                    \
        if ((fp = fopen(path, "r"))) {   \
            fscanf(fp, format, dest);    \
            fclose(fp);                  \
        }                                \
    }
#define file_put(fp, path, format, value) \
    {                                     \
        if ((fp = fopen(path, "w+"))) {   \
            fprintf(fp, format, value);   \
            fclose(fp);                   \
        }                                 \
    }
#define file_put_sync(fp, path, format, value) \
    {                                          \
        if ((fp = fopen(path, "w+"))) {        \
            fprintf(fp, format, value);        \
            fflush(fp);                        \
            fsync(fileno(fp));                 \
            fclose(fp);                        \
        }                                      \
    }

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define CONTENT_INT "%d"
#define CONTENT_STR "%[^\n]"

bool exists(const char *file_path);
bool is_file(const char *file_path);
bool is_dir(const char *file_path);
bool file_isModified(const char *path, time_t *old_mtime);
bool file_isLocked(const char *path);

/**
 * @brief returns the filename component of a path
 * 
 * This is a copy of the GNU `basename` version and
 * retains all the quirks that come along with it
 * 
 * See 'Versions' here:
 * https://man7.org/linux/man-pages/man3/basename.3.html
 * 
 * Copied from: 
 * https://sourceware.org/git/?p=glibc.git;a=blob;f=string/basename.c;h=d5b5d4763dd3fa307497cc99788b0bb24c95bcf1;hb=refs/heads/master#l22
 * 
 * @param filename The full file path.
 * @return * char* 
 */
const char *file_basename(const char *filename);

/**
 * @brief Create directories in dir_path using `mkdir -p` command.
 *
 * @param dir_path The full directory path.
 * @return true If the path didn't exist (dirs were created).
 * @return false If the path exists (no dirs were created).
 */
bool mkdirs(const char *dir_path);

void file_readLastLine(const char *filename, char *out_str);

char *file_read(const char *path) __attribute__((malloc));

bool file_write(const char *path, const char *str, uint32_t len);

void file_copy(const char *src_path, const char *dest_path);

char *file_removeExtension(const char *myStr) __attribute__((malloc));

char *file_dirname(const char *absolutePath) __attribute__((malloc));

void file_cleanName(char *name_out, const char *file_name);

const char *file_getExtension(const char *filename);

char *file_parseKeyValue(const char *file_path, const char *key_in,
                         char *value_out, char divider, int select_index);

void file_changeKeyValue(const char *file_path, const char *key,
                         const char *replacement_line);

bool file_path_relative_to(char *path_out, const char *path_from, const char *path_to);

bool file_findNewest(const char *dir_path, char *newest_file, size_t buffer_size);

FILE *file_open_ensure_path(const char *path, const char *mode);

char *file_read_lineN(const char *filename, int n) __attribute__((malloc));

void file_delete_line(const char *fileName, int n);

void file_add_line_to_beginning(const char *filename, const char *lineToAdd);

/**
 * @brief Resolve a path to an absolute path
 * 
 * "/mnt/SDCARD/Emu/GBA/../../Roms/GBA/game.gba" -> "/mnt/SDCARD/Roms/GBA/game.gba"
 *
 */
char *file_resolvePath(const char *path) __attribute__((malloc));

#endif // UTILS_FILE_H__
