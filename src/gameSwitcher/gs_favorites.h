#ifndef GAME_SWITCHER_FAVORITES_H__
#define GAME_SWITCHER_FAVORITES_H__

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "components/JsonGameEntry.h"
#include "utils/file.h"
#include "utils/json.h"
#include "utils/log.h"

#include "gs_model.h"

// FAVORITES_PATH comes from components/JsonGameEntry.h, which is the canonical
// definition and also documents the on-disk entry format.
#define FAVORITES_TMP_PATTERN FAVORITES_PATH ".tmp.XXXXXX"

static bool _favorites_pathMatches(const char *stored_rompath,
                                   const char *rompath,
                                   const char *resolved_rompath)
{
    if (stored_rompath == NULL || rompath == NULL) {
        return false;
    }

    char *resolved_stored = file_resolvePath(stored_rompath);
    bool matches = resolved_stored != NULL && resolved_rompath != NULL
                       ? strcmp(resolved_stored, resolved_rompath) == 0
                       : strcmp(stored_rompath, rompath) == 0;

    free(resolved_stored);
    return matches;
}

static bool _favorites_lineMatches(const char *line,
                                   const char *rompath,
                                   const char *resolved_rompath)
{
    cJSON *json = cJSON_Parse(line);
    if (json == NULL) {
        return false;
    }

    cJSON *stored_rompath = cJSON_GetObjectItemCaseSensitive(json, "rompath");
    bool matches = cJSON_IsString(stored_rompath) &&
                   stored_rompath->valuestring != NULL &&
                   _favorites_pathMatches(stored_rompath->valuestring, rompath, resolved_rompath);

    cJSON_Delete(json);
    return matches;
}

static bool favorites_contains(Game_s *game)
{
    FILE *file = fopen(FAVORITES_PATH, "r");
    if (file == NULL) {
        return false;
    }

    const char *rompath = game->recentItem.rompath;
    char *resolved_rompath = file_resolvePath(rompath);
    char *line = NULL;
    size_t capacity = 0;
    bool found = false;

    while (getline(&line, &capacity, file) != -1) {
        if (_favorites_lineMatches(line, rompath, resolved_rompath)) {
            found = true;
            break;
        }
    }

    free(line);
    free(resolved_rompath);
    fclose(file);
    return found;
}

static bool _favorites_writeItem(FILE *file, const RecentItem *item)
{
    bool success = false;
    cJSON *json = cJSON_CreateObject();
    if (json == NULL) {
        return false;
    }

    if (cJSON_AddStringToObject(json, "label", item->label) == NULL ||
        cJSON_AddStringToObject(json, "rompath", item->rompath) == NULL ||
        cJSON_AddStringToObject(json, "imgpath", item->imgpath) == NULL ||
        cJSON_AddStringToObject(json, "launch", item->launch) == NULL ||
        cJSON_AddNumberToObject(json, "type", item->type) == NULL) {
        goto cleanup;
    }

    char *serialized = cJSON_PrintUnformatted(json);
    if (serialized != NULL) {
        success = fprintf(file, "%s\n", serialized) >= 0;
        cJSON_free(serialized);
    }

cleanup:
    cJSON_Delete(json);
    return success;
}

static bool favorites_toggle(Game_s *game, bool *is_favorite_after)
{
    FILE *source = fopen(FAVORITES_PATH, "r");
    if (source == NULL && errno != ENOENT) {
        print_debug("Could not open favorites file");
        return false;
    }

    struct stat source_stat;
    bool source_stat_valid = stat(FAVORITES_PATH, &source_stat) == 0;

    char temp_path[] = FAVORITES_TMP_PATTERN;
    int temp_fd = mkstemp(temp_path);
    if (temp_fd < 0) {
        if (source != NULL) {
            fclose(source);
        }
        print_debug("Could not create temporary favorites file");
        return false;
    }

    fchmod(temp_fd, source_stat_valid ? source_stat.st_mode & 0777 : 0644);

    FILE *target = fdopen(temp_fd, "w");
    if (target == NULL) {
        close(temp_fd);
        unlink(temp_path);
        if (source != NULL) {
            fclose(source);
        }
        print_debug("Could not open temporary favorites file");
        return false;
    }

    const char *rompath = game->recentItem.rompath;
    char *resolved_rompath = file_resolvePath(rompath);
    char *line = NULL;
    size_t capacity = 0;
    bool was_favorite = false;
    bool success = true;

    while (source != NULL && getline(&line, &capacity, source) != -1) {
        if (_favorites_lineMatches(line, rompath, resolved_rompath)) {
            was_favorite = true;
            continue;
        }

        size_t line_length = strlen(line);
        if (fwrite(line, 1, line_length, target) != line_length ||
            (line_length > 0 && line[line_length - 1] != '\n' && fputc('\n', target) == EOF)) {
            success = false;
            break;
        }
    }

    if (source != NULL && ferror(source)) {
        success = false;
    }
    if (source != NULL) {
        fclose(source);
    }
    free(line);
    free(resolved_rompath);

    if (success && !was_favorite) {
        success = _favorites_writeItem(target, &game->recentItem);
    }
    if (success && fflush(target) != 0) {
        success = false;
    }
    if (success && fsync(fileno(target)) != 0) {
        success = false;
    }
    if (fclose(target) != 0) {
        success = false;
    }

    if (!success || rename(temp_path, FAVORITES_PATH) != 0) {
        unlink(temp_path);
        print_debug("Could not update favorites file");
        return false;
    }

    sync();
    if (is_favorite_after != NULL) {
        *is_favorite_after = !was_favorite;
    }
    return true;
}

#endif // GAME_SWITCHER_FAVORITES_H__
