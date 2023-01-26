#ifndef DEVICE_MODEL_H__
#define DEVICE_MODEL_H__

#include <stdio.h>
#include "utils/file.h"

#define MIYOO283 283
#define MIYOO353 353

static int DEVICE_ID;

/**
 * @brief Get device model
 * MM = Miyoo mini 
 * MMP = Miyoo mini plus
 */
 
void getDeviceModel(void) {
    FILE *fp;
    file_get(fp, "/tmp/deviceModel", "%d", &DEVICE_ID);
}

#endif // DEVICE_MODEL_H__
