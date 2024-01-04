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
    int old_percentage = -1, current_percentage, warn_at = 15, last_logged_percentage = -1;

    atexit(cleanup);
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);
    signal(SIGSTOP, sigHandler);
    signal(SIGCONT, sigHandler);
    signal(SIGUSR1, sigHandler);

    display_init();
    int ticks = CHECK_BATTERY_TIMEOUT_S;

    bool is_charging = false;

    while (!quit) {
        if (battery_isCharging()) {
            if (!is_charging) {
                // Charging just started
                is_charging = true;
                if (DEVICE_ID == MIYOO354) {
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

            printf_debug(
                "Charging stopped: suspended = %d, perc = %d, warn = %d\n",
                is_suspended, current_percentage, warn_at);

            if (DEVICE_ID == MIYOO283) {
                adc_value_g = updateADCValue(0);
                current_percentage = batteryPercentage(adc_value_g);
                saveFakeAxpResult(current_percentage);
            }
            else if (DEVICE_ID == MIYOO354) {
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
                }
                else if (DEVICE_ID == MIYOO354) {
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
                file_put_sync(fp, "/tmp/percBat", "%d", current_percentage);

                if (abs(last_logged_percentage - current_percentage) >= BATTERY_LOG_THRESHOLD) {
                    // Current battery state duration addition
                    update_current_duration();
                    // New battery percentage entry
                    log_new_percentage(current_percentage, is_charging);
                    last_logged_percentage = current_percentage;
                }

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
        else if (current_percentage < warn_at && !config_flag_get(".noBatteryWarning") && !process_isRunning("MainUI")) {
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
    display_free();
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

    if (value <= 100)
        value = adcConfig.adc_value;
    else if (adcConfig.adc_value > value)
        value++;
    else if (adcConfig.adc_value < value)
        value--;

    return value;
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

int batteryPercentage(int value)
{
    if (value == 100)
        return 500;
    if (value >= 578)
        return 100;
    if (value >= 528)
        return value - 478;
    if (value >= 512)
        return (int)(value * 2.125 - 1068);
    if (value >= 480)
        return (int)(value * 0.51613 - 243.742);
    return 0;
}

static void *batteryWarning_thread(void *param)
{
    while (1) {
        display_drawBatteryIcon(0x00FF0000, 15, RENDER_HEIGHT - 30, 10,
                                0x00FF0000); // draw red battery icon
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
