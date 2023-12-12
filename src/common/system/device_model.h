#ifndef DEVICE_MODEL_H__
#define DEVICE_MODEL_H__

#include "utils/file.h"
#include <stdio.h>

#define MIYOO283 283
#define MIYOO354 354

static int DEVICE_ID;
static char DEVICE_SN[12];

/**
 * @brief Get device model
 * MM = Miyoo mini
 * MMP = Miyoo mini plus
 */

void getDeviceModel(void)
{
    FILE *fp;
    file_get(fp, "/tmp/deviceModel", "%d", &DEVICE_ID);
}

void getDeviceSerial(void)
{
    FILE *fp;
    file_get(fp, "/tmp/deviceSN", "%[^\n]", DEVICE_SN);
}

#endif // DEVICE_MODEL_H__
