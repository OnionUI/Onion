#ifndef ADC_H__
#define ADC_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <pthread.h>

#ifdef PLATFORM_MIYOOMINI
#include "shmvar/shmvar.h"
#endif

#include "system.h"
#include "utils.h"
#include "display.h"
#include "settings.h"

// for reading battery
#define SARADC_IOC_MAGIC 'a'
#define IOCTL_SAR_INIT _IO(SARADC_IOC_MAGIC, 0)
#define IOCTL_SAR_SET_CHANNEL_READ_VALUE _IO(SARADC_IOC_MAGIC, 1)
typedef struct {
    int channel_value;
    int adc_value;
} SAR_ADC_CONFIG_READ;

static SAR_ADC_CONFIG_READ adcCfg = {0,0};
static int sar_fd = 0;
static int adc_value = 0;
static bool adcthread_active;
static pthread_t adc_pt;


int battery_readADC(void) {
    char charging = 0;
    char filename[128];
    const char str_export[] = "59";
    const char str_direction[] = "in";
    int fd;

    concat(filename, GPIO_DIR2, "gpio59/value");
    fd = open(filename, O_RDONLY);
    if (fd < 0) {
        // export gpio59, direction: in
        concat(filename, GPIO_DIR1, "export");
        fd = open(filename, O_WRONLY); if (fd >= 0) { write(fd, str_export, 2); close(fd); }
        concat(filename, GPIO_DIR2, "gpio59/direction");
        fd = open(filename, O_WRONLY); if (fd >= 0) { write(fd, str_direction, 2); close(fd); }
        concat(filename, GPIO_DIR2, "gpio59/value");
        fd = open(filename, O_RDONLY);
    }
    if (fd >= 0) { read(fd, &charging, 1); close(fd); }
    if (charging == '1') {
        adc_value = 100;
        return adc_value;
    }

    if (!sar_fd) {
        sar_fd = open("/dev/sar", O_WRONLY);
        ioctl(sar_fd, IOCTL_SAR_INIT, NULL);
    }

    ioctl(sar_fd, IOCTL_SAR_SET_CHANNEL_READ_VALUE, &adcCfg);

    if (adc_value <= 100) adc_value = adcCfg.adc_value;
    else if ( adcCfg.adc_value > adc_value ) adc_value++;
    else if ( adcCfg.adc_value < adc_value ) adc_value--;

    return adc_value;
}

void battery_monitorADC_on(void)
{
    if (!sar_fd) {
        sar_fd = open("/dev/sar", O_WRONLY);
        ioctl(sar_fd, IOCTL_SAR_INIT, NULL);
    }
    
    ioctl(sar_fd, IOCTL_SAR_SET_CHANNEL_READ_VALUE, &adcCfg);

    #ifdef PLATFORM_MIYOOMINI
    KeyShmInfo info;
    InitKeyShm(&info);
    SetKeyShm(&info, MONITOR_ADC_VALUE, adcCfg.adc_value);
    UninitKeyShm(&info);
    #endif
}

void battery_monitorADC_off(void)
{
    #ifdef PLATFORM_MIYOOMINI
    KeyShmInfo info;
    InitKeyShm(&info);
    SetKeyShm(&info, MONITOR_ADC_VALUE, 640);
    UninitKeyShm(&info);
    #endif
}

int battery_readPercentage() {
    battery_readADC();
    if (adc_value == 100) return 500;
    if (adc_value >= 578) return 100;
    if (adc_value >= 528) return adc_value - 478;
    if (adc_value >= 512) return (int)(adc_value * 2.125 - 1068);
    if (adc_value >= 480) return (int)(adc_value * 0.51613 - 243.742);
    return 0;
}

int battery_getPercentage()
{
    int percentage = 0;

    if (file_exists("/tmp/percBat")) {
        char val[5];
        const char *cPercBat = file_readAll("/tmp/percBat");
        strcpy(val, cPercBat);
        percentage = atoi(val);
    }
    #ifdef PLATFORM_MIYOOMINI
    else {
        percentage = battery_readPercentage();
    }
    #endif

    return percentage;
}

//
//    Draw Battery warning thread
//
static void* battery_warning_thread(void* param) {
    while (1) {
        display_drawFrame(0x00FF0000); // draw red frame
        usleep(0x4000);
    }
    return 0;
}

//
//    Check Battery warning
//
bool battery_isLow(void) {
    if (adc_value < 506 && adc_value != 100)
        return true;
    return false;
}

void battery_showWarning(void)
{
    if (adcthread_active || !settings.low_battery_warning)
        return;
    pthread_create(&adc_pt, NULL, battery_warning_thread, NULL);
    adcthread_active = true;
}

void battery_hideWarning(void)
{
    if (!adcthread_active)
        return;
    pthread_cancel(adc_pt);
    pthread_join(adc_pt, NULL);
    display_drawFrame(0); // erase red frame
    adcthread_active = false;
}

//
//    Update ADC Value
//
void battery_updateADC(bool reset) {
    if (reset) adc_value = 0; // reset ADC value
    battery_readADC();
    if (battery_isLow())
        battery_showWarning();
    else
        battery_hideWarning();
}

void battery_free(void) {
    if (sar_fd)
        close(sar_fd);
}

#endif // ADC_H__
