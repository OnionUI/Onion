#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include "sys/ioctl.h"

typedef struct {
    int channel_value;
    int adc_value;
} SAR_ADC_CONFIG_READ;

#define SARADC_IOC_MAGIC                     'a'
#define IOCTL_SAR_INIT                       _IO(SARADC_IOC_MAGIC, 0)
#define IOCTL_SAR_SET_CHANNEL_READ_VALUE     _IO(SARADC_IOC_MAGIC, 1)

static SAR_ADC_CONFIG_READ  adcCfg = {0,0};
static int sar_fd = 0;


static int is_charging = 0;
void checkCharging(void) {
    int i = 0;
    FILE *file = fopen("/sys/devices/gpiochip0/gpio/gpio59/value", "r");
    if (file!=NULL) {
        fscanf(file, "%i", &i);
        fclose(file);
    }
    is_charging = i;
}


int main (int argc, char *argv[]) {
   
	checkCharging();
	if (is_charging == 1){
		int adc_fd = open("/tmp/.isCharging", O_CREAT | O_WRONLY);
			if (adc_fd > 0) {
   				close(adc_fd); 
			}
	}

}
