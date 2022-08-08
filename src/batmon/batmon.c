#include "batmon.h"

int main(int argc, char *argv[])
{
    FILE *fp;
    int old_percentage = -1,
        current_percentage,
        warn_at = 15;

    atexit(cleanup);
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    // TODO: implement SUSPEND/RESUME handlers - show/hide warning - reset adc_value on resume

    sar_fd = open("/dev/sar", O_WRONLY);
    ioctl(sar_fd, IOCTL_SAR_INIT, NULL);
    ioctl(sar_fd, IOCTL_SAR_SET_CHANNEL_READ_VALUE, &adcConfig);

    display_init();

    while (!quit) {
        config_get("battery/warnAt", "%d", &warn_at);

        adc_value = updateADCValue(adc_value);
        current_percentage = batteryPercentage(adc_value);

        if (current_percentage != old_percentage) {
            file_put_sync(fp, "/tmp/percBat", "%d", current_percentage);

            #ifdef PLATFORM_MIYOOMINI
            if (current_percentage <= warn_at && !config_flag_get(".noBatteryWarning"))
                batteryWarning_show();
            else
                batteryWarning_hide();
            #endif

            old_percentage = current_percentage;
        }

        msleep(CHECK_BATTERY_TIMEOUT);
    }

    close(sar_fd);
}

static void sigHandler(int sig)
{
    quit = true;
    msleep_interrupt = 1;
}

void cleanup(void)
{
    remove("/tmp/percBat");
    display_free();
}

bool isCharging(void)
{
    char charging = 0;
    int fd = open(GPIO_DIR2 "gpio59/value", O_RDONLY);

    if (fd < 0) {
        // export gpio59, direction: in
        file_write(GPIO_DIR1 "export", "59", 2);
        file_write(GPIO_DIR2 "gpio59/direction", "in", 2);
        fd = open(GPIO_DIR2 "gpio59/value", O_RDONLY);
    }

    if (fd >= 0) {
        read(fd, &charging, 1);
        close(fd);        
    }

    return charging == '1';
}

int updateADCValue(int adc_value)
{    
    if (isCharging())
        return 100;

    if (adc_value <= 100)
        adc_value = adcConfig.adc_value;
    else if (adcConfig.adc_value > adc_value)
        adc_value++;
    else if (adcConfig.adc_value < adc_value)
        adc_value--;

    return adc_value;
}

int batteryPercentage(int adc_value)
{
    if (adc_value == 100) return 500;
    if (adc_value >= 578) return 100;
    if (adc_value >= 528) return adc_value - 478;
    if (adc_value >= 512) return (int)(adc_value * 2.125 - 1068);
    if (adc_value >= 480) return (int)(adc_value * 0.51613 - 243.742);
    return 0;
}

//
//    Draw Battery warning thread
//
static void* batteryWarning_thread(void* param)
{
    while (1) {
        display_drawFrame(0x00FF0000); // draw red frame
        usleep(0x4000);
    }
    return 0;
}

void batteryWarning_show(void)
{
    if (adcthread_active)
        return;
    pthread_create(&adc_pt, NULL, batteryWarning_thread, NULL);
    adcthread_active = true;
}

void batteryWarning_hide(void)
{
    if (!adcthread_active)
        return;
    pthread_cancel(adc_pt);
    pthread_join(adc_pt, NULL);
    display_drawFrame(0); // erase red frame
    adcthread_active = false;
}
