#ifndef KEYMON_INPUT_FD_H__
#define KEYMON_INPUT_FD_H__

#include <linux/fb.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <sys/poll.h>

#include "utils/msleep.h"

// for ev.value
#define RELEASED 0
#define PRESSED 1
#define REPEAT 2

// for button_flag
#define SELECT_BIT 0
#define START_BIT 1
#define L2_BIT 2
#define R2_BIT 3
#define SELECT (1 << SELECT_BIT)
#define START (1 << START_BIT)
#define L2 (1 << L2_BIT)
#define R2 (1 << R2_BIT)

#define QUEUE_MAX 10

// Global Variables
static int input_fd;
static struct input_event ev;
static struct pollfd fds[1];
static bool keyinput_disabled = false;
static int ignore_queue[QUEUE_MAX][2];
static int ignore_queue_count = 0;

void _ignoreQueue_remove(int index)
{
    printf_debug("Removing from ignore queue: code=%d, value=%d\n",
                 ignore_queue[index][0], ignore_queue[index][1]);
    for (int i = index; i < ignore_queue_count - 1; i++) {
        ignore_queue[i][0] = ignore_queue[i + 1][0];
        ignore_queue[i][1] = ignore_queue[i + 1][1];
    }
    ignore_queue_count--;
}

void _ignoreQueue_add(int code, int value)
{
    if (ignore_queue_count >= QUEUE_MAX)
        return;
    printf_debug("Adding to ignore queue: code=%d, value=%d\n", code, value);
    ignore_queue[ignore_queue_count][0] = code;
    ignore_queue[ignore_queue_count][1] = value;
    ignore_queue_count++;
}

bool keyinput_isValid(void)
{
    read(input_fd, &ev, sizeof(ev));

    if (ev.type != EV_KEY || ev.value > REPEAT)
        return false;

    for (int i = 0; i < ignore_queue_count; i++) {
        if (ignore_queue[i][0] == ev.code && ignore_queue[i][1] == ev.value) {
            _ignoreQueue_remove(i);
            return false;
        }
    }

    return true;
}

void keyinput_send(unsigned short code, signed int value)
{
    if (keyinput_disabled)
        return;
    char cmd[100];
    sprintf(cmd, "sendkeys %d %d", code, value);
    printf_debug("Send keys: code=%d, value=%d\n", code, value);
    _ignoreQueue_add(code, value);
    system(cmd);
    print_debug("Keys sent");
}

void keyinput_sendMulti(int n, int code_value_pairs[n][2])
{
    if (keyinput_disabled)
        return;
    char cmd[512];
    strcpy(cmd, "./bin/sendkeys ");

    for (int i = 0; i < n; i++) {
        int code = code_value_pairs[i][0];
        int value = code_value_pairs[i][1];
        _ignoreQueue_add(code, value);
        sprintf(cmd + strlen(cmd), "%d %d ", code, value);
    }

    printf_debug("Send keys: %s\n", cmd);
    system(cmd);
    print_debug("Keys sent");
}

/**
 * @brief stop input event for other processes
 *
 */
void keyinput_disable(void)
{
    if (keyinput_disabled)
        return;
    while (ioctl(input_fd, EVIOCGRAB, 1) < 0) {
        usleep(100000);
    }
    keyinput_disabled = true;
    print_debug("Keyinput disabled");
}

/**
 * @brief restart input event for other processes
 *
 */
void keyinput_enable(void)
{
    if (!keyinput_disabled)
        return;
    while (ioctl(input_fd, EVIOCGRAB, 0) < 0) {
        usleep(100000);
    }
    keyinput_disabled = false;
    print_debug("Keyinput enabled");
}

#endif // KEYMON_INPUT_FD_H__
