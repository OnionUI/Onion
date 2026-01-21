
#ifndef SQLMON_H
#define SQLMON_H

#include "../playActivity/cacheDB.h"
#include "../playActivity/playActivityDBCommon.h"
#include <arpa/inet.h>
#include <dirent.h>
#include <libgen.h>
#include <limits.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <sqlite3/sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

#include "utils/log.h"
#include "utils/msleep.h"

static sqlite3 *play_activity_db_reader = NULL;
static sqlite3 *play_activity_db_writer = NULL;

static int quit = 0;

static void sigHandler(int sig)
{
    switch (sig) {
    case SIGINT:
    case SIGTERM:
        quit = 1;
        break;
    default:
        break;
    }
}
#endif
