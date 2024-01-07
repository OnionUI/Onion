#ifndef ADC_H__
#define ADC_H__

#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <sqlite3/sqlite3.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <unistd.h>

#ifdef PLATFORM_MIYOOMINI
#include "shmvar/shmvar.h"
#endif

#include "system/battery.h"
#include "system/display.h"
#include "system/system.h"
#include "utils/config.h"
#include "utils/file.h"
#include "utils/flags.h"
#include "utils/log.h"

#include "batmonDB.h"

#define CHECK_BATTERY_TIMEOUT_S 15 // s - check battery percentage every 15s

// Battery logs
#define BATTERY_LOG_THRESHOLD 2 // Define when a new battery entry is logged
#define FILO_MIN_SIZE 1000
#define MAX_DURATION_BEFORE_UPDATE 600

// for reading battery
#define SARADC_IOC_MAGIC 'a'
#define IOCTL_SAR_INIT _IO(SARADC_IOC_MAGIC, 0)
#define IOCTL_SAR_SET_CHANNEL_READ_VALUE _IO(SARADC_IOC_MAGIC, 1)

typedef struct {
    int channel_value;
    int adc_value;
} SAR_ADC_CONFIG_READ;

static bool adcthread_active = false;
static pthread_t adc_pt;
static bool quit = false;
static int sar_fd, adc_value_g;
static bool is_suspended = false;

static void sigHandler(int sig);
void cleanup(void);

void update_current_duration(void);
void log_new_percentage(int new_bat_value, int is_charging);
int get_current_session_time(void);
int set_best_session_time(int best_session);
void saveFakeAxpResult(int current_percentage);
bool isCharging(void);
int updateADCValue(int);
int getBatPercMMP(void);
int batteryPercentage(int);
static void *batteryWarning_thread(void *param);
void batteryWarning_show(void);
void batteryWarning_hide(void);
bool warningDisabled(void);

#endif // ADC_H__
