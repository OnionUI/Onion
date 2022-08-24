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
    signal(SIGSTOP, sigHandler);
    signal(SIGCONT, sigHandler);

    display_init();

    int ticks = CHECK_BATTERY_TIMEOUT_S;
    bool is_charging = false;

    while (!quit) {
        if (battery_isCharging()) {
            is_charging = true;
            current_percentage = 500;
        }
        else if (is_charging) {
            is_charging = false;
            adc_value_g = updateADCValue(0);
            current_percentage = batteryPercentage(adc_value_g);
            printf_debug("charging stopped: suspended = %d, perc = %d, warn = %d\n", is_suspended, current_percentage, warn_at);
        }

        if (!is_suspended) {
            config_get("battery/warnAt", "%d", &warn_at);

            if (ticks >= CHECK_BATTERY_TIMEOUT_S) {
                adc_value_g = updateADCValue(adc_value_g);
                current_percentage = batteryPercentage(adc_value_g);
                printf_debug("battery check: suspended = %d, perc = %d, warn = %d\n", is_suspended, current_percentage, warn_at);
                ticks = -1;
            }
            
            if (current_percentage != old_percentage) {
                printf_debug("saving percBat: suspended = %d, perc = %d, warn = %d\n", is_suspended, current_percentage, warn_at);
                old_percentage = current_percentage;
                file_put_sync(fp, "/tmp/percBat", "%d", current_percentage);
            }
        }
        else {
            ticks = -1;
        }

        #ifdef PLATFORM_MIYOOMINI
        if (is_suspended || current_percentage == 500)
            batteryWarning_hide();
        else if (current_percentage < warn_at && !config_flag_get(".noBatteryWarning"))
            batteryWarning_show();
        else
            batteryWarning_hide();
        #endif

        ticks++;
        sleep(1);
    }

    return EXIT_SUCCESS;
}

static void sigHandler(int sig)
{
    switch (sig) {
        case SIGINT:
        case SIGTERM:
            quit = true;
            msleep_interrupt = 1;
            break;
        case SIGSTOP:
            is_suspended = true;
            break;
        case SIGCONT:
            adc_value_g = updateADCValue(0);
            is_suspended = false;
            break;
        default: break;
    }
}

void cleanup(void)
{
    remove("/tmp/percBat");
    display_free();
    close(sar_fd);
}

int updateADCValue(int value)
{    
    if (battery_isCharging())
        return 100;

    if (!sar_fd) {
        sar_fd = open("/dev/sar", O_WRONLY);
        ioctl(sar_fd, IOCTL_SAR_INIT, NULL);
    }

    static SAR_ADC_CONFIG_READ adcConfig;
    ioctl(sar_fd, IOCTL_SAR_SET_CHANNEL_READ_VALUE, &adcConfig);

    if (value <= 100) value = adcConfig.adc_value;
    else if (adcConfig.adc_value > value) value++;
    else if (adcConfig.adc_value < value) value--;

    return value;
}

int batteryPercentage(int value)
{
    if (value == 100) return 500;
    if (value >= 578) return 100;
    if (value >= 528) return value - 478;
    if (value >= 512) return (int)(value * 2.125 - 1068);
    if (value >= 480) return (int)(value * 0.51613 - 243.742);
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
