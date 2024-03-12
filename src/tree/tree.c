/*  The MIT License (MIT)
*   
*   Copyright (c) 2016-present Kevin Newton
*   
*   Permission is hereby granted, free of charge, to any person obtaining a copy
*   of this software and associated documentation files (the "Software"), to deal
*   in the Software without restriction, including without limitation the rights
*   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*   copies of the Software, and to permit persons to whom the Software is
*   furnished to do so, subject to the following conditions:
*   
*   The above copyright notice and this permission notice shall be included in
*   all copies or substantial portions of the Software.
*   
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*   THE SOFTWARE.
*/

// taken from https://github.com/kddnewton/tree/blob/main/tree.c and modified to fit our needs

#define _GNU_SOURCE

#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct counter {
    size_t dirs;
    size_t files;
} counter_t;

typedef struct entry {
    char *name;
    int is_dir;
    struct entry *next;
} entry_t;

bool is_extension_included(const char *filename,
                           const char *included_extensions[])
{
    if (!included_extensions) // No included extensions specified
        return true;

    const char *extension = strrchr(filename, '.');
    if (!extension)
        return false; // File without extension, exclude

    extension++; // Move past the dot
    for (int i = 0; included_extensions[i] != NULL; i++)
        if (strcmp(extension, included_extensions[i]) == 0)
            return true; // Included extension found

    return false; // No included extension found
}

int is_directory_excluded(const char *directory_name,
                          const char *excluded_directories[])
{
    if (!excluded_directories) // No excluded directories specified
        return false;

    for (int i = 0; excluded_directories[i] != NULL; i++)
        if (strcmp(directory_name, excluded_directories[i]) == 0)
            return true; // Excluded directory found

    return false;
}

int tree(const char *directory, const char *prefix, counter_t *counter,
         const char *included_extensions[],
         const char *excluded_directories[])
{
    entry_t *head = NULL, *current, *iter;
    size_t size = 0, index;

    struct dirent *file_dirent;
    DIR *dir_handle;

    char *full_path, *segment, *pointer, *next_prefix;

    dir_handle = opendir(directory);
    if (!dir_handle) {
        fprintf(stderr, "Cannot open directory \"%s\"\n", directory);
        return -1;
    }

    counter->dirs++;

    while ((file_dirent = readdir(dir_handle)) != NULL) {
        if (strcmp(file_dirent->d_name, ".") == 0 || strcmp(file_dirent->d_name, "..") == 0) // no . or ..
            continue;

        if (file_dirent->d_type == DT_DIR &&
            is_directory_excluded(file_dirent->d_name, excluded_directories))
            continue;

        if (file_dirent->d_type == DT_REG &&
            !is_extension_included(file_dirent->d_name, included_extensions))
            continue;

        current = malloc(sizeof(entry_t));
        current->name =
            strcpy(malloc(strlen(file_dirent->d_name) + 1), file_dirent->d_name);
        current->is_dir = file_dirent->d_type == DT_DIR;
        current->next = NULL;

        if (head == NULL) {
            head = current;
        }
        else if (strcmp(current->name, head->name) < 0) {
            current->next = head;
            head = current;
        }
        else {
            for (iter = head;
                 iter->next && strcmp(current->name, iter->next->name) > 0;
                 iter = iter->next)
                ;

            current->next = iter->next;
            iter->next = current;
        }

        size++;
    }

    closedir(dir_handle);

    if (!head) { // no entries at all
        free(head);
        return 0;
    }

    // print list
    for (index = 0; index < size; index++) {
        if (index == size - 1) {
            pointer = "└── ";
            segment = "    ";
        }
        else {
            pointer = "├── ";
            segment = "│   ";
        }

        printf("%s%s%s\n", prefix, pointer, head->name);

        if (head->is_dir) {
            full_path = malloc(strlen(directory) + strlen(head->name) + 2);
            sprintf(full_path, "%s/%s", directory, head->name);

            next_prefix = malloc(strlen(prefix) + strlen(segment) + 1);
            sprintf(next_prefix, "%s%s", prefix, segment);

            tree(full_path, next_prefix, counter, included_extensions,
                 excluded_directories);
            free(full_path);
            free(next_prefix);
        }
        else {
            counter->files++;
        }

        current = head;
        head = head->next;

        free(current->name);
        free(current);
    }

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(
            stderr,
            "Usage: %s <directory> [-e \"directory_name1 directory_name2 ...\"] "
            "[-i \"ext1 ext2 ...\"]\n",
            argv[0]);
        return 1;
    }

    char *directory = argv[1];
    printf("%s\n", directory);

    counter_t counter = {0, 0};
    const char **excluded_directories = NULL;
    const char **included_extensions = NULL;

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-e") == 0) {
            if (i + 1 < argc) {
                char *directories = argv[++i];
                char *token = strtok(directories, " ");
                int count = 0;
                while (token != NULL) {
                    excluded_directories =
                        realloc(excluded_directories, (count + 2) * sizeof(char *));
                    excluded_directories[count++] = token;
                    excluded_directories[count] = NULL;
                    token = strtok(NULL, " ");
                }
            }
            else {
                fprintf(stderr, "Error: Missing argument for -e\n");
                return 1;
            }
        }
        else if (strcmp(argv[i], "-i") == 0) {
            if (i + 1 < argc) {
                char *extensions = argv[++i];
                char *token = strtok(extensions, " ");
                int count = 0;
                while (token != NULL) {
                    included_extensions =
                        realloc(included_extensions, (count + 2) * sizeof(char *));
                    included_extensions[count++] = token;
                    included_extensions[count] = NULL;
                    token = strtok(NULL, " ");
                }
            }
            else {
                fprintf(stderr, "Error: Missing argument for -i\n");
                return 1;
            }
        }
    }

    tree(directory, "", &counter, included_extensions, excluded_directories);

    printf("\n%zu directories, %zu files\n", counter.dirs ? counter.dirs - 1 : 0,
           counter.files);

    if (excluded_directories)
        free(excluded_directories);
    if (included_extensions)
        free(included_extensions);

    return 0;
}
