#ifndef ADC_H__
#define ADC_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include "shmvar/shmvar.h"

#include "utils.h"

//    for read battery
#define SARADC_IOC_MAGIC                     'a'
#define IOCTL_SAR_INIT                       _IO(SARADC_IOC_MAGIC, 0)
#define IOCTL_SAR_SET_CHANNEL_READ_VALUE     _IO(SARADC_IOC_MAGIC, 1)
typedef struct {
    int channel_value;
    int adc_value;
} SAR_ADC_CONFIG_READ;

#define    GPIO_DIR1 "/sys/class/gpio/"
#define    GPIO_DIR2 "/sys/devices/gpiochip0/gpio/"

static SAR_ADC_CONFIG_READ adcCfg = {0,0};
static int sar_fd = 0;

int adc_getValue(void) {
    int adc_value = 0;
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

void adc_monitorOn(void)
{
    if (!sar_fd) {
        sar_fd = open("/dev/sar", O_WRONLY);
        ioctl(sar_fd, IOCTL_SAR_INIT, NULL);
    }
    
    ioctl(sar_fd, IOCTL_SAR_SET_CHANNEL_READ_VALUE, &adcCfg);

    KeyShmInfo info;
    InitKeyShm(&info);
    SetKeyShm(&info, MONITOR_ADC_VALUE, adcCfg.adc_value);
    UninitKeyShm(&info);
}

void adc_monitorOff(void)
{
    KeyShmInfo info;
    InitKeyShm(&info);
    SetKeyShm(&info, MONITOR_ADC_VALUE, 640);
    UninitKeyShm(&info);
}

int adc_batteryPercentage(int adc_value) {
    if (adc_value == 100) return 500;
    if (adc_value >= 578) return 100;
    if (adc_value >= 528) return adc_value - 478;
    if (adc_value >= 512) return (int)(adc_value * 2.125 - 1068);
    if (adc_value >= 480) return (int)(adc_value * 0.51613 - 243.742);
    return 0;
}

int getBatteryPercentage()
{
    int percentage = 0;

    if (file_exists("/tmp/percBat")) {
        char val[5];
        const char *cPercBat = file_readAll("/tmp/percBat");
        strcpy(val, cPercBat);
        percentage = atoi(val);
    }
    else {
        int adcvalue = adc_getValue();
        percentage = adc_batteryPercentage(adcvalue);
    }

    return percentage;
}

#endif // ADC_H__
