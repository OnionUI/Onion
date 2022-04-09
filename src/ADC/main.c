#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include "sys/ioctl.h"
#include <linux/input.h>

typedef struct {
    int channel_value;
    int adc_value;
} SAR_ADC_CONFIG_READ;
 
#define SARADC_IOC_MAGIC                     'a'
#define IOCTL_SAR_INIT                       _IO(SARADC_IOC_MAGIC, 0)
#define IOCTL_SAR_SET_CHANNEL_READ_VALUE     _IO(SARADC_IOC_MAGIC, 1)

static SAR_ADC_CONFIG_READ  adcCfg = {0,0};
static int sar_fd = 0;

static void initADC(void) {
    sar_fd = open("/dev/sar", O_WRONLY);
    ioctl(sar_fd, IOCTL_SAR_INIT, NULL);
}

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


void rumble(uint32_t val) {
	int fd;
	const char str_export[] = "48";
	const char str_direction[] = "out";
	char value[1];
	value[0] = ((val&1)^1) + 0x30;

	fd = open("/sys/class/gpio/export",O_WRONLY);
		if (fd > 0) {
			write(fd, str_export, 2);
			close(fd);
		}
	fd = open("/sys/class/gpio/gpio48/direction",O_WRONLY);
		if (fd > 0) {
			write(fd, str_direction, 3);
			close(fd);
		}
	fd = open("/sys/class/gpio/gpio48/value",O_WRONLY);
		if (fd > 0) {
			write(fd, value, 1);
			close(fd);
		}
}


int percBat = 0;
int firstLaunch = 1; 
int countChecks = 0;

static void checkADC(void) {  
    ioctl(sar_fd, IOCTL_SAR_SET_CHANNEL_READ_VALUE, &adcCfg);
    remove("/tmp/percBat"); 
    int adc_fd = open("/tmp/percBat", O_CREAT | O_WRONLY);
	
	int old_is_charging = is_charging;
	checkCharging();
	
    if (adc_fd>0) {
        char val[3];
                
  		int percBatTemp = 0;
  		if (is_charging == 0){
			if (adcCfg.adc_value >= 528){
  				percBatTemp = adcCfg.adc_value-478;
  			}
  			else if ((adcCfg.adc_value >= 512) && (adcCfg.adc_value < 528)){
  				percBatTemp = (int)(adcCfg.adc_value*2.125-1068);		
  			}
  			else if ((adcCfg.adc_value >= 480) && (adcCfg.adc_value < 512)){
  				percBatTemp = (int)(adcCfg.adc_value* 0.51613 - 243.742);		
  			}
  			
  			if ((firstLaunch == 1) || (old_is_charging == 1)){
        		// Calibration needed at first launch or when the 
        		// user just unplugged his charger
        		firstLaunch = 0;
        		percBat =  percBatTemp;
        	}
        	else {
        		if (percBat>percBatTemp){
  					percBat--;
  				}
  				else if (percBat < percBatTemp){
  					percBat++;
  				}        
        	}
        	if (percBat<0){
 				percBat=0;
 			}	
  			if (percBat>100){
 				percBat=100;
 			}	

  		}
  		else {
  		// The handheld is currently charging
  		percBat = 500 ;
  		}
  
  		
        sprintf(val, "%d", percBat);
        write(adc_fd, val, strlen(val)); 
        close(adc_fd); 
        
        // Rumble in case of low battery
  		if (percBat<=10){
  			rumble(1);
			usleep(3000000);	//3s
			rumble(0);		
  		}
  		
  		if (percBat<=4){
			// Force retroarch to end its session
			
			int fd = creat(".deepSleep", 777);
			close(fd);	
			system("killall -15 retroarch");
  		}
  		
  		countChecks ++;
    }
}

static void* runADC(void *arg) {
    while(1) {
        checkADC();
        sleep(15);
     
    }
    return 0;
}

int main (int argc, char *argv[]) {
	/*
	sleep(15);
	//sleep(10); 10 seconds

	int input_fd = open("/dev/input/event0", O_WRONLY);
	struct input_event	ev;

	ev.type = EV_KEY;
	ev.code = KEY_SPACE;
	ev.value = 1;		// Press

	write(input_fd, &ev, sizeof(ev));
	close(input_fd);
*/
	
	
	initADC();   
	runADC(NULL);
}
