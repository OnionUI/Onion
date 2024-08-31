#ifndef JSON_H__
#define JSON_H__

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "./file.h"
#include "cjson/cJSON.h"

#define JSON_STRING_LEN 256
#define JSON_FORMAT_NUMBER "    \"%s\": %d,\n"
#define JSON_FORMAT_NUMBER_NC "    \"%s\": %d\n"
#define JSON_FORMAT_STRING "    \"%s\": \"%s\",\n"
#define JSON_FORMAT_STRING_NC "    \"%s\": \"%s\"\n"
#define JSON_FORMAT_TAB_NUMBER "	\"%s\":	%d,\n"
#define JSON_FORMAT_TAB_NUMBER_NC "	\"%s\":	%d\n"
#define JSON_FORMAT_TAB_STRING "	\"%s\":	\"%s\",\n"
#define JSON_FORMAT_TAB_STRING_NC "	\"%s\":	\"%s\"\n"

bool json_getString(cJSON *object, const char *key, char *dest)
{
    cJSON *json_object = cJSON_GetObjectItem(object, key);
    if (json_object) {
        strncpy(dest, cJSON_GetStringValue(json_object), JSON_STRING_LEN - 1);
        return true;
    }
    return false;
}

bool json_getBool(cJSON *object, const char *key, bool *dest)
{
    cJSON *json_object = cJSON_GetObjectItem(object, key);
    if (json_object) {
        *dest = cJSON_IsTrue(json_object);
        return true;
    }
    return false;
}

bool json_getInt(cJSON *object, const char *key, int *dest)
{
    cJSON *json_object = cJSON_GetObjectItem(object, key);
    if (json_object) {
        *dest = (int)cJSON_GetNumberValue(json_object);
        return true;
    }
    return false;
}

bool json_getDouble(cJSON *object, const char *key, double *dest)
{
    cJSON *json_object = cJSON_GetObjectItem(object, key);
    if (json_object) {
        *dest = cJSON_GetNumberValue(json_object);
        return true;
    }
    return false;
}

bool json_setString(cJSON *object, const char *key, const char *value)
{
    cJSON *json_object = cJSON_GetObjectItem(object, key);
    if (json_object) {
        cJSON_SetValuestring(json_object, value);
        return true;
    }
    return false;
}

bool json_forceSetString(cJSON *object, const char *key, const char *value)
{
    cJSON *json_object = cJSON_GetObjectItem(object, key);

    if (json_object) {
        cJSON_SetValuestring(json_object, value);
        return true;
    }

    return cJSON_AddStringToObject(object, key, value);
}

/**
 * @brief Loads and parses a json file.
 *
 * @param file_path
 * @return cJSON* Root json object. Remember to cJSON_Delete the result.
 */
cJSON *json_load(const char *file_path)
{
    char *file_contents = file_read(file_path);
    cJSON *json_contents = cJSON_Parse(file_contents);
    free(file_contents);
    return json_contents;
}

void json_save(cJSON *object, char *file_path)
{
    if (object == NULL || file_path == NULL)
        return;

    char *output = cJSON_Print(object);

    FILE *fp = NULL;
    if ((fp = fopen(file_path, "w+")) != NULL) {
        fwrite(output, strlen(output), 1, fp);
        fclose(fp);
    }

    if (output != NULL)
        cJSON_free(output);
}

#endif // JSON_H__
