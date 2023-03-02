#ifndef PROCESS_H__
#define PROCESS_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>

#ifndef DT_DIR
#define DT_DIR 4
#endif

//
//    Search pid of running executable (forward match)
//
pid_t process_searchpid(const char *commname)
{
    DIR *procdp;
    struct dirent *dir;
    char fname[24];
    char comm[128];
    pid_t pid;
    pid_t ret = 0;
    size_t commlen = strlen(commname);

    procdp = opendir("/proc");
    while ((dir = readdir(procdp))) {
        if (dir->d_type == DT_DIR) {
            pid = atoi(dir->d_name);
            if ( pid > 2 ) {
                sprintf(fname, "/proc/%d/comm", pid);
                FILE *fp = fopen(fname, "r");
                if (fp) {
                    fscanf(fp, "%127s", comm);
                    fclose(fp);
                    if (!strncmp(comm, commname, commlen)) { ret = pid; break; }
                }
            }
        }
    }
    closedir(procdp);
    return ret;
}

bool process_isRunning(const char *commname)
{
    return process_searchpid(commname) != 0;
}

void process_kill(const char *commname)
{
    pid_t pid;
    if ((pid = process_searchpid(commname)))
        kill(pid, SIGKILL);
}

void process_killall(const char *commname)
{
    pid_t pid;
    int max = 999;
    while ((pid = process_searchpid(commname)) && max-- > 0)
        kill(pid, SIGKILL);
}

bool process_start(const char *pname, const char *args, const char *home, bool await)
{
    char filename[256]; sprintf(filename, "%s/bin/%s", home != NULL ? home : ".", pname);
    if (!exists(filename)) sprintf(filename, "%s/%s", home != NULL ? home : ".", pname);
    if (!exists(filename)) sprintf(filename, "/mnt/SDCARD/.tmp_update/bin/%s", pname);
    if (!exists(filename)) sprintf(filename, "/mnt/SDCARD/.tmp_update/%s", pname);
    if (!exists(filename)) sprintf(filename, "/mnt/SDCARD/miyoo/app/%s", pname);
    if (!exists(filename)) return false;

    char cmd[512];
    sprintf(cmd, "cd \"%s\"; %s %s %s",
        home != NULL ? home : ".",
        filename,
        args != NULL ? args : "",
        await ? "" : "&");
    system(cmd);

    return true;
}

#endif // PROCESS_H__
