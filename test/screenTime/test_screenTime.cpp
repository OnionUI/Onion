#include "gtest/gtest.h"

#include <sqlite3/sqlite3.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

extern "C" {
#include "../../src/screenTime/screenTime.h"
}

class TempDir {
  public:
    TempDir()
    {
        char tmpl[] = "/tmp/onion-screen-time-test-XXXXXX";
        char *created = mkdtemp(tmpl);
        path_ = created == nullptr ? "" : created;
    }

    ~TempDir()
    {
        if (!path_.empty()) {
            std::string cmd = "rm -rf \"" + path_ + "\"";
            system(cmd.c_str());
        }
    }

    const std::string &path() const { return path_; }

  private:
    std::string path_;
};

class ScopedEnv {
  public:
    ScopedEnv(const char *name, const std::string &value)
        : name_(name)
    {
        const char *previous = getenv(name);
        if (previous != nullptr) {
            had_previous_ = true;
            previous_ = previous;
        }

        setenv(name_.c_str(), value.c_str(), 1);
    }

    ~ScopedEnv()
    {
        if (had_previous_)
            setenv(name_.c_str(), previous_.c_str(), 1);
        else
            unsetenv(name_.c_str());
    }

  private:
    std::string name_;
    bool had_previous_ = false;
    std::string previous_;
};

static void execSql(sqlite3 *db, const char *sql)
{
    char *err = nullptr;
    ASSERT_EQ(sqlite3_exec(db, sql, nullptr, nullptr, &err), SQLITE_OK) << (err == nullptr ? "" : err);
    sqlite3_free(err);
}

static void insertActivity(sqlite3 *db, int64_t created_at, const char *play_time)
{
    char sql[256];
    snprintf(sql, sizeof(sql),
             "INSERT INTO play_activity(rom_id, play_time, created_at, updated_at) "
             "VALUES(1, %s, %lld, %s);",
             play_time, (long long)created_at, play_time);
    execSql(db, sql);
}

static std::string createActivityDb(const std::string &dir)
{
    std::string dbPath = dir + "/play_activity_db.sqlite";
    sqlite3 *db = nullptr;
    EXPECT_EQ(sqlite3_open(dbPath.c_str(), &db), SQLITE_OK);
    execSql(db,
            "CREATE TABLE play_activity("
            "rom_id INTEGER, "
            "play_time INTEGER, "
            "created_at INTEGER, "
            "updated_at INTEGER);");
    sqlite3_close(db);
    return dbPath;
}

TEST(test_screenTime, missingDatabaseHasZeroUsage)
{
    int64_t used = -1;
    ASSERT_EQ(screen_time_get_usage_seconds("/tmp/does-not-exist-screen-time.sqlite", 1700000000, &used), 0);
    ASSERT_EQ(used, 0);
}

TEST(test_screenTime, dailyUsageCountsOnlyTodayOverlap)
{
    TempDir dir;
    ASSERT_FALSE(dir.path().empty());
    std::string dbPath = createActivityDb(dir.path());

    time_t now = 1700000000;
    time_t dayStart = 0;
    time_t dayEnd = 0;
    screen_time_today_bounds(now, &dayStart, &dayEnd);

    sqlite3 *db = nullptr;
    ASSERT_EQ(sqlite3_open(dbPath.c_str(), &db), SQLITE_OK);

    insertActivity(db, dayStart + 3600, "600");
    insertActivity(db, dayStart - 600, "1200");
    insertActivity(db, dayEnd - 300, "900");
    insertActivity(db, dayStart - 7200, "3600");
    insertActivity(db, dayStart + 5000, "-10");
    insertActivity(db, now - 120, "NULL");

    sqlite3_close(db);

    ScopedEnv bootTime("SCREEN_TIME_BOOT_TIME", std::to_string((long long)dayStart));

    int64_t used = 0;
    ASSERT_EQ(screen_time_get_usage_seconds(dbPath.c_str(), now, &used), 0);
    ASSERT_EQ(used, 1620);
}

TEST(test_screenTime, activeRowsBeforeBootAreIgnored)
{
    TempDir dir;
    ASSERT_FALSE(dir.path().empty());
    std::string dbPath = createActivityDb(dir.path());

    time_t now = 1700000000;
    time_t dayStart = 0;
    time_t dayEnd = 0;
    screen_time_today_bounds(now, &dayStart, &dayEnd);

    sqlite3 *db = nullptr;
    ASSERT_EQ(sqlite3_open(dbPath.c_str(), &db), SQLITE_OK);
    insertActivity(db, dayStart + 100, "NULL");
    insertActivity(db, now - 120, "NULL");
    sqlite3_close(db);

    ScopedEnv bootTime("SCREEN_TIME_BOOT_TIME", std::to_string((long long)(now - 300)));

    int64_t used = 0;
    ASSERT_EQ(screen_time_get_usage_seconds(dbPath.c_str(), now, &used), 0);
    ASSERT_EQ(used, 120);
}

TEST(test_screenTime, dailyBoundsResolveDstAtMidnight)
{
    time_t now = 1710072000;
    time_t dayStart = 0;
    time_t dayEnd = 0;

    screen_time_today_bounds(now, &dayStart, &dayEnd);

    struct tm localStart;
    localtime_r(&dayStart, &localStart);
    ASSERT_EQ(localStart.tm_hour, 0);
    ASSERT_EQ(localStart.tm_min, 0);
    ASSERT_EQ(localStart.tm_sec, 0);
    ASSERT_GT(dayEnd, dayStart);
}

TEST(test_screenTime, statusIncludesMatchingDayExtraTime)
{
    TempDir dir;
    ASSERT_FALSE(dir.path().empty());
    std::string dbPath = createActivityDb(dir.path());

    time_t now = 1700000000;
    time_t dayStart = 0;
    time_t dayEnd = 0;
    screen_time_today_bounds(now, &dayStart, &dayEnd);

    sqlite3 *db = nullptr;
    ASSERT_EQ(sqlite3_open(dbPath.c_str(), &db), SQLITE_OK);
    insertActivity(db, dayStart + 60, "600");
    sqlite3_close(db);

    std::string configDir = dir.path() + "/config";
    ASSERT_EQ(mkdir(configDir.c_str(), 0777), 0);

    std::string nowStr = std::to_string((long long)now);
    setenv("SCREEN_TIME_ACTIVITY_DB", dbPath.c_str(), 1);
    setenv("SCREEN_TIME_CONFIG_DIR", configDir.c_str(), 1);
    setenv("SCREEN_TIME_NOW", nowStr.c_str(), 1);

    FILE *fp = fopen((configDir + "/enabled").c_str(), "w");
    ASSERT_NE(fp, nullptr);
    fputs("1", fp);
    fclose(fp);

    fp = fopen((configDir + "/dailyLimitMinutes").c_str(), "w");
    ASSERT_NE(fp, nullptr);
    fputs("10", fp);
    fclose(fp);

    ASSERT_EQ(screen_time_add_extra_minutes(5), 0);

    ScreenTimeStatus status;
    ASSERT_EQ(screen_time_get_status(&status), 0);
    ASSERT_TRUE(status.enabled);
    ASSERT_EQ(status.used_seconds, 600);
    ASSERT_EQ(status.limit_seconds, 600);
    ASSERT_EQ(status.extra_seconds, 300);
    ASSERT_EQ(status.remaining_seconds, 300);
    ASSERT_TRUE(screen_time_launch_allowed(&status));

    unsetenv("SCREEN_TIME_ACTIVITY_DB");
    unsetenv("SCREEN_TIME_CONFIG_DIR");
    unsetenv("SCREEN_TIME_NOW");
}

TEST(test_screenTime, settingsSettersPersistValues)
{
    TempDir dir;
    ASSERT_FALSE(dir.path().empty());
    std::string configDir = dir.path() + "/config";

    time_t now = 1700000000;
    std::string nowStr = std::to_string((long long)now);
    setenv("SCREEN_TIME_CONFIG_DIR", configDir.c_str(), 1);
    setenv("SCREEN_TIME_NOW", nowStr.c_str(), 1);

    ASSERT_EQ(screen_time_set_enabled(true), 0);
    ASSERT_EQ(screen_time_set_daily_limit_minutes(45), 0);
    ASSERT_EQ(screen_time_set_extra_minutes(15), 0);

    ScreenTimeSettings settings;
    ASSERT_EQ(screen_time_load_settings(&settings), 0);
    ASSERT_TRUE(settings.enabled);
    ASSERT_EQ(settings.daily_limit_minutes, 45);
    ASSERT_EQ(settings.extra_minutes, 15);

    char today[SCREEN_TIME_DATE_LEN];
    screen_time_date_string(now, today, sizeof(today));
    ASSERT_STREQ(settings.extra_date, today);

    unsetenv("SCREEN_TIME_CONFIG_DIR");
    unsetenv("SCREEN_TIME_NOW");
}

TEST(test_screenTime, debugRemainingOverrideCountsDownWhenEnabled)
{
    TempDir dir;
    ASSERT_FALSE(dir.path().empty());
    std::string dbPath = createActivityDb(dir.path());
    std::string configDir = dir.path() + "/config";

    time_t now = 1700000000;
    std::string nowStr = std::to_string((long long)now);
    setenv("SCREEN_TIME_ACTIVITY_DB", dbPath.c_str(), 1);
    setenv("SCREEN_TIME_CONFIG_DIR", configDir.c_str(), 1);
    setenv("SCREEN_TIME_NOW", nowStr.c_str(), 1);

    ASSERT_EQ(screen_time_set_enabled(true), 0);
    ASSERT_EQ(screen_time_set_daily_limit_minutes(60), 0);
    ASSERT_EQ(screen_time_debug_set_remaining_seconds(15), 0);

    ScreenTimeStatus status;
    ASSERT_EQ(screen_time_get_status(&status), 0);
    ASSERT_TRUE(status.enabled);
    ASSERT_EQ(status.remaining_seconds, 15);
    ASSERT_TRUE(screen_time_launch_allowed(&status));

    nowStr = std::to_string((long long)(now + 15));
    setenv("SCREEN_TIME_NOW", nowStr.c_str(), 1);
    ASSERT_EQ(screen_time_get_status(&status), 0);
    ASSERT_EQ(status.remaining_seconds, 0);
    ASSERT_FALSE(screen_time_launch_allowed(&status));
    ASSERT_NE(access((configDir + "/debugExpireAt").c_str(), F_OK), 0);

    ASSERT_EQ(screen_time_get_status(&status), 0);
    ASSERT_EQ(status.remaining_seconds, 3600);

    unsetenv("SCREEN_TIME_ACTIVITY_DB");
    unsetenv("SCREEN_TIME_CONFIG_DIR");
    unsetenv("SCREEN_TIME_NOW");
}

TEST(test_screenTime, pinIsHashedAndVerified)
{
    TempDir dir;
    ASSERT_FALSE(dir.path().empty());
    std::string configDir = dir.path() + "/config";
    setenv("SCREEN_TIME_CONFIG_DIR", configDir.c_str(), 1);

    ASSERT_FALSE(screen_time_pin_configured());
    ASSERT_TRUE(screen_time_verify_pin("1234"));

    ASSERT_EQ(screen_time_set_pin("parent passphrase"), 0);
    ASSERT_TRUE(screen_time_pin_configured());
    ASSERT_TRUE(screen_time_verify_pin("parent passphrase"));
    ASSERT_FALSE(screen_time_verify_pin("0000"));

    FILE *fp = fopen((configDir + "/pinHash").c_str(), "r");
    ASSERT_NE(fp, nullptr);
    char stored[64] = {0};
    ASSERT_NE(fgets(stored, sizeof(stored), fp), nullptr);
    fclose(fp);
    ASSERT_STRNE(stored, "parent passphrase");

    ASSERT_EQ(screen_time_set_pin(""), 0);
    ASSERT_FALSE(screen_time_pin_configured());

    unsetenv("SCREEN_TIME_CONFIG_DIR");
}
