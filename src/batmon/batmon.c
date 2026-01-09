#include "batmon.h"
#include "system/device_model.h"
#include "utils/process.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int battery_current_state_duration = 0;
int best_session_time = 0;

int main(int argc, char *argv[])
{
    log_setName("batmon");
    getDeviceModel();
    getDeviceSerial();
    best_session_time = get_best_session_time();

    FILE *fp;
    int old_percentage = -1, current_percentage = 0, warn_at = 15;
    int lowest_percentage_after_charge = 500;
    atexit(cleanup);
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);
    signal(SIGSTOP, sigHandler);
    signal(SIGCONT, sigHandler);
    signal(SIGUSR1, sigHandler);

    display_init(true);
    int ticks = CHECK_BATTERY_TIMEOUT_S;

    bool is_charging = false;

    while (!quit) {
        if (battery_isCharging()) {
            if (!is_charging) {
                // Charging just started
                lowest_percentage_after_charge = 500; // Reset lowest percentage before charge
                is_charging = true;
                if (IS_MIYOO_PLUS_OR_FLIP()) {
                    current_percentage = getBatPercMMP();
                    // To solve : Sometimes getBatPercMMP returns 1735289191
                    current_percentage = (current_percentage > 100) ? old_percentage : current_percentage;
                }
                else {
                    current_percentage = 500;
                    saveFakeAxpResult(current_percentage);
                }
                update_current_duration();

                int session_time = get_current_session_time();
                printf_debug("Charging detected - Previous session duration = %d\n", session_time);

                if (session_time > best_session_time) {
                    printf_debug("Best session duration\n", 1);
                    set_best_session_time(session_time);
                    best_session_time = session_time;
                }
                log_new_percentage(current_percentage, is_charging);
            }
        }
        else if (is_charging) {
            // Charging just stopped
            is_charging = false;
            lowest_percentage_after_charge = 500; // Reset lowest percentage after charge

            printf_debug(
                "Charging stopped: suspended = %d, perc = %d, warn = %d\n",
                is_suspended, current_percentage, warn_at);

            if (DEVICE_ID == MIYOO283) {
                adc_value_g = updateADCValue(0);
                current_percentage = batteryPercentage(adc_value_g);
                saveFakeAxpResult(current_percentage);
            }
            else if (IS_MIYOO_PLUS_OR_FLIP()) {
                current_percentage = getBatPercMMP();
            }
            update_current_duration();
            log_new_percentage(current_percentage, is_charging);
        }

        if (!is_suspended) {
            config_get("battery/warnAt", CONFIG_INT, &warn_at);

            if (ticks >= CHECK_BATTERY_TIMEOUT_S) {
                if (DEVICE_ID == MIYOO283) {
                    adc_value_g = updateADCValue(adc_value_g);
                    current_percentage = batteryPercentage(adc_value_g);
                    // Avvoid battery increasing from tiny voltage changes, assume lowest until it drops below it.
                    // This is better than assuming the max cpu and screen power consumption and trying to calculate the battery drain.
                    if (current_percentage < lowest_percentage_after_charge) {
                        lowest_percentage_after_charge = current_percentage;
                    }
                    else {
                        current_percentage = lowest_percentage_after_charge;
                    }
                }
                else if (IS_MIYOO_PLUS_OR_FLIP()) {
                    current_percentage = getBatPercMMP();
                    // To solve : Sometimes getBatPercMMP returns 1735289191
                    current_percentage = (current_percentage > 100) ? old_percentage : current_percentage;
                }
                printf_debug(
                    "battery check: suspended = %d, perc = %d, warn = %d\n",
                    is_suspended, current_percentage, warn_at);

                ticks = -1;
            }

            if (current_percentage != old_percentage) {
                // This statement is not englobed in the previous one
                // in order to be launched once when batmon starts
                printf_debug(
                    "saving percBat: suspended = %d, perc = %d, warn = %d\n",
                    is_suspended, current_percentage, warn_at);
                old_percentage = current_percentage;
                // Save battery percentage to file
                file_put_sync(fp, "/tmp/percBat", "%d", current_percentage);
                // Current battery state duration addition
                update_current_duration();
                // New battery percentage entry
                log_new_percentage(current_percentage, is_charging);
                if (DEVICE_ID == MIYOO283) {
                    saveFakeAxpResult(current_percentage);
                }
            }
        }
        else {
            ticks = -1;
        }

#ifdef PLATFORM_MIYOOMINI
        if (is_suspended || current_percentage == 500) {
            batteryWarning_hide();
        }
        else if (current_percentage < warn_at && !warningDisabled()) {
            batteryWarning_show();
        }
        else {
            batteryWarning_hide();
        }
#endif
        if (battery_current_state_duration > MAX_DURATION_BEFORE_UPDATE)
            update_current_duration();

        sleep(1);
        battery_current_state_duration++;
        ticks++;
    }

    // Current battery state duration addition
    update_current_duration();
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
        if (DEVICE_ID == MIYOO283) {
            adc_value_g = updateADCValue(0);
        }
        is_suspended = false;
        break;
    case SIGUSR1:
        display_getRenderResolution();
        break;
    default:
        break;
    }
}

void cleanup(void)
{
    remove("/tmp/percBat");
    display_close();
    close(sar_fd);
}

void update_current_duration(void)
{
    if (open_battery_log_db() == 1) {
        if (bat_log_db != NULL) {
            const char *sql = "SELECT * FROM bat_activity WHERE device_serial = ? ORDER BY id DESC LIMIT 1;";
            sqlite3_stmt *stmt;
            int rc = sqlite3_prepare_v2(bat_log_db, sql, -1, &stmt, 0);

            if (rc == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1, DEVICE_SN, -1, SQLITE_STATIC);
                rc = sqlite3_step(stmt);
                if (rc == SQLITE_ROW) {
                    int current_duration = sqlite3_column_int(stmt, 3);
                    int new_duration = current_duration + battery_current_state_duration;

                    const char *update_sql = "UPDATE bat_activity SET duration = ? WHERE id = ?";

                    sqlite3_stmt *update_stmt;
                    rc = sqlite3_prepare_v2(bat_log_db, update_sql, -1, &update_stmt, 0);

                    if (rc == SQLITE_OK) {
                        sqlite3_bind_int(update_stmt, 1, new_duration);
                        sqlite3_bind_int(update_stmt, 2, sqlite3_column_int(stmt, 0));

                        // Exécuter la mise à jour
                        rc = sqlite3_step(update_stmt);

                        battery_current_state_duration = 0;
                        sqlite3_finalize(stmt);
                        sqlite3_finalize(update_stmt);
                    }
                }
            }
            close_battery_log_db();
        }
    }
}

void log_new_percentage(int new_bat_value, int is_charging)
{
    if (open_battery_log_db() == 1) {
        if (bat_log_db != NULL) {
            char *sql = sqlite3_mprintf("INSERT INTO bat_activity(device_serial, bat_level, duration, is_charging) VALUES(%Q, %d, %d, %d);", DEVICE_SN, new_bat_value, 0, is_charging);
            sqlite3_exec(bat_log_db, sql, NULL, NULL, NULL);
            sqlite3_free(sql);

            // FILO logic
            sqlite3_stmt *stmt;
            const char *count_sql = "SELECT COUNT(id) FROM bat_activity";

            int count = 0;

            if (sqlite3_prepare_v2(bat_log_db, count_sql, -1, &stmt, NULL) == SQLITE_OK) {
                if (sqlite3_step(stmt) == SQLITE_ROW) {
                    count = sqlite3_column_int(stmt, 0);
                }
                sqlite3_finalize(stmt);
            }

            if (count > FILO_MIN_SIZE) {
                // Deletion of the 1st entry
                const char *delete_sql = "DELETE FROM bat_activity WHERE id = (SELECT MIN(id) FROM bat_activity);";
                sqlite3_prepare_v2(bat_log_db, delete_sql, -1, &stmt, 0);
                sqlite3_step(stmt);
                sqlite3_finalize(stmt);
            }
        }
        close_battery_log_db();
    }
}

int get_current_session_time(void)
{
    int current_session_duration = 0;
    if (open_battery_log_db() == 1) {
        if (bat_log_db != NULL) {
            const char *sql = "SELECT * FROM bat_activity WHERE device_serial = ? AND is_charging = 1 ORDER BY id DESC LIMIT 1;";
            sqlite3_stmt *stmt;
            int rc = sqlite3_prepare_v2(bat_log_db, sql, -1, &stmt, 0);

            if (rc == SQLITE_OK) {

                sqlite3_bind_text(stmt, 1, DEVICE_SN, -1, SQLITE_STATIC);
                rc = sqlite3_step(stmt);
                if (rc == SQLITE_ROW) {

                    sqlite3_stmt *stmt_sum;
                    const char *query_sum = "SELECT SUM(duration) FROM bat_activity WHERE device_serial = ? AND id > ?;";
                    rc = sqlite3_prepare_v2(bat_log_db, query_sum, -1, &stmt_sum, NULL);
                    if (rc == SQLITE_OK) {
                        sqlite3_bind_text(stmt_sum, 1, DEVICE_SN, -1, SQLITE_STATIC);
                        sqlite3_bind_int(stmt_sum, 2, sqlite3_column_int(stmt, 0));
                        while ((rc = sqlite3_step(stmt_sum)) == SQLITE_ROW) {
                            current_session_duration = sqlite3_column_int(stmt_sum, 0);
                        }
                    }
                    sqlite3_finalize(stmt_sum);
                }
            }
            sqlite3_finalize(stmt);
        }
        close_battery_log_db();
    }
    return current_session_duration;
}

int set_best_session_time(int best_session)
{
    int is_success = 0;
    if (open_battery_log_db() == 1) {
        if (bat_log_db != NULL) {
            const char *sql = "SELECT * FROM device_specifics WHERE device_serial = ? ORDER BY id LIMIT 1;";
            sqlite3_stmt *stmt;
            int rc = sqlite3_prepare_v2(bat_log_db, sql, -1, &stmt, 0);

            if (rc == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1, DEVICE_SN, -1, SQLITE_STATIC);
                rc = sqlite3_step(stmt);
                if (rc == SQLITE_ROW) {
                    const char *update_sql = "UPDATE device_specifics SET best_session = ? WHERE id = ?";

                    sqlite3_stmt *update_stmt;
                    rc = sqlite3_prepare_v2(bat_log_db, update_sql, -1, &update_stmt, 0);

                    if (rc == SQLITE_OK) {
                        sqlite3_bind_int(update_stmt, 1, best_session);
                        sqlite3_bind_int(update_stmt, 2, sqlite3_column_int(stmt, 0));

                        // Exécuter la mise à jour
                        rc = sqlite3_step(update_stmt);

                        sqlite3_finalize(stmt);
                        sqlite3_finalize(update_stmt);
                        is_success = 1;
                    }
                }
            }
            close_battery_log_db();
        }
    }
    return is_success;
}

void saveFakeAxpResult(int current_percentage)
{
    FILE *fp;
    if ((fp = fopen("/tmp/.axp_result", "w+"))) {
        fprintf(fp, "{\"battery\":%d, \"voltage\":%d, \"charging\":%d}", current_percentage, adc_value_g, current_percentage == 500 ? 3 : 0);
        fflush(fp);
        fsync(fileno(fp));
        fclose(fp);
    }
}

#define HISTORY_SIZE 5 // Number of values to smooth battery percentage

int updateADCValue(int value)
{
    if (battery_isCharging())
        return 100;

    if (!sar_fd) {
        sar_fd = open("/dev/sar", O_WRONLY);
        ioctl(sar_fd, IOCTL_SAR_INIT, NULL);
    }

    static SAR_ADC_CONFIG_READ adcConfig;
    static int history[HISTORY_SIZE] = {0}; // Array to store history
    static int historyIndex = 0;            // Current index in the history
    static int historyCount = 0;            // Number of valid entries in history

    ioctl(sar_fd, IOCTL_SAR_SET_CHANNEL_READ_VALUE, &adcConfig);

    // Update history buffer
    history[historyIndex] = adcConfig.adc_value;
    historyIndex = (historyIndex + 1) % HISTORY_SIZE; // Wrap around
    if (historyCount < HISTORY_SIZE) {
        historyCount++;
    }

    // Calculate the average of the history
    int sum = 0;
    for (int i = 0; i < historyCount; i++) {
        sum += history[i];
    }
    int smoothedValue = sum / historyCount;

    return smoothedValue;
}

int getBatPercMMP(void)
{
    char buf[100] = "";
    int battery_number;

    system("cd /customer/app/ ; ./axp_test > /tmp/.axp_result");

    FILE *fp;
    file_get(fp, "/tmp/.axp_result", CONTENT_STR, buf);
    sscanf(buf, "{\"battery\":%d, \"voltage\":%*d, \"charging\":%*d}", &battery_number);

    return battery_number;
}

typedef struct {
    float voltage;
    float percentage;
} VoltagePercentMapping;

VoltagePercentMapping VoltageCurveMapping_liion[] = {
    {4.20, 100.0}, // super accurate fresh miyoom mini battery curve mapping data.
    {4.13, 95.0},
    {4.06, 90.0},
    {4.02, 85.0},
    {3.99, 80.0},
    {3.95, 75.0},
    {3.92, 70.0},
    {3.88, 65.0},
    {3.85, 60.0},
    {3.81, 55.0},
    {3.78, 50.0},
    {3.74, 45.0},
    {3.71, 40.0},
    {3.67, 35.0},
    {3.64, 30.0},
    {3.56, 25.0},
    {3.49, 20.0},
    {3.42, 15.0},
    {3.30, 10.0},
    {3.14, 5.0},
    {3.00, 0.0}};

float adcToVoltage(int adcValue)
{
    return (adcValue - 148) * 0.01;
}

float interpolatePercentage(float voltage)
{
    // Lookup table size
    int table_size = sizeof(VoltageCurveMapping_liion) / sizeof(VoltageCurveMapping_liion[0]);

    // Interpolation logic
    for (int i = 0; i < table_size - 1; i++) {
        if (voltage <= VoltageCurveMapping_liion[i].voltage && voltage > VoltageCurveMapping_liion[i + 1].voltage) {
            // Perform linear interpolation
            float voltage_diff = VoltageCurveMapping_liion[i].voltage - VoltageCurveMapping_liion[i + 1].voltage;
            float percentage_diff = VoltageCurveMapping_liion[i].percentage - VoltageCurveMapping_liion[i + 1].percentage;
            float voltage_offset = voltage - VoltageCurveMapping_liion[i + 1].voltage;
            return VoltageCurveMapping_liion[i + 1].percentage + (voltage_offset / voltage_diff) * percentage_diff;
        }
    }
    // If voltage is outside the lookup table, see if it's above or below
    if (voltage >= VoltageCurveMapping_liion[0].voltage)
        return 100;
    if (voltage <= VoltageCurveMapping_liion[table_size - 1].voltage)
        return 0;

    return -1; // Error
}

int batteryPercentage(int adcValue)
{
    if (adcValue == 100) // Charging
        return 500;

    if (config_flag_get("battery/.isLegacyType")) {
        if (adcValue >= 578)
            return 100;
        if (adcValue >= 528)
            return adcValue - 478;
        if (adcValue >= 512)
            return (int)(adcValue * 2.125 - 1068);
        if (adcValue >= 480)
            return (int)(adcValue * 0.51613 - 243.742);
        return 0;
    }

    // Convert ADC value to voltage
    float voltage = adcToVoltage(adcValue);

    // unfinished but un-needed screen brightness vdrop adjustment, keeping incase needed
    // float screen_voltage_drop = 0.06;                        // 0.06 drop when screen is on max brightness
    // float brightness = display_getBrightnessFromRaw() * 10; // 0 - 100
    // voltage -= screen_voltage_drop * (brightness / 100);    // Adjust voltage for screen brightness, linear drop

    // Interpolate battery percentage from voltage
    float percentage = interpolatePercentage(voltage);

    return (int)percentage; //display direct for now
}

static void *batteryWarning_thread(void *param)
{
    while (1) {
        if (temp_flag_get("hasBatteryDisplay"))
            break;
        if (display_getBrightnessRaw() != 0) {
            display_drawBatteryIcon(0x00FF0000, 15, g_display.height - 30, 10,
                                    0x00FF0000); // draw red battery icon
        }
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
    adcthread_active = false;
}

bool warningDisabled(void)
{
    return config_flag_get(".noBatteryWarning") || temp_flag_get("hasBatteryDisplay") || process_isRunning("MainUI");
}
