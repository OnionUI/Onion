#ifndef JSON_H__
#define JSON_H__

#include <stdbool.h>
#include <string.h>

#include "cjson/cJSON.h"

#define JSON_STRING_LEN 256
#define JSON_FORMAT_NUMBER    "    \"%s\": %d,\n"
#define JSON_FORMAT_NUMBER_NC "    \"%s\": %d\n"
#define JSON_FORMAT_STRING    "    \"%s\": \"%s\",\n"
#define JSON_FORMAT_STRING_NC "    \"%s\": \"%s\"\n"

bool json_string(cJSON* root, const char* key, char* dest)
{
    cJSON* json_object = cJSON_GetObjectItem(root, key);
    if (json_object) {
        strncpy(dest, cJSON_GetStringValue(json_object), JSON_STRING_LEN - 1);
        return true;
    }
    return false;
}

bool json_bool(cJSON* root, const char* key, bool* dest)
{
    cJSON* json_object = cJSON_GetObjectItem(root, key);
    if (json_object) {
        *dest = cJSON_IsTrue(json_object);
        return true;
    }
    return false;
}

bool json_int(cJSON* root, const char* key, int* dest)
{
    cJSON* json_object = cJSON_GetObjectItem(root, key);
    if (json_object) {
        *dest = (int)cJSON_GetNumberValue(json_object);
        return true;
    }
    return false;
}

bool json_double(cJSON* root, const char* key, double* dest)
{
    cJSON* json_object = cJSON_GetObjectItem(root, key);
    if (json_object) {
        *dest = cJSON_GetNumberValue(json_object);
        return true;
    }
    return false;
}

#endif // JSON_H__
