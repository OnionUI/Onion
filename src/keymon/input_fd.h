#ifndef KEYMON_INPUT_FD_H__
#define KEYMON_INPUT_FD_H__

#include <sys/ioctl.h>
#include <linux/fb.h>
#include <linux/input.h>

// for ev.value
#define RELEASED 0
#define PRESSED  1
#define REPEAT   2

// for button_flag
#define SELECT_BIT 0
#define START_BIT  1
#define L2_BIT     2
#define R2_BIT     3
#define SELECT     (1<<SELECT_BIT)
#define START      (1<<START_BIT)
#define L2         (1<<L2_BIT)
#define R2         (1<<R2_BIT)

// Global Variables
static int input_fd;
static struct input_event ev;
static struct pollfd fds[1];
static bool keyinput_disabled = false;

/**
 * @brief stop input event for other processes
 * 
 */
void keyinput_disable(void)
{
    while (ioctl(input_fd, EVIOCGRAB, 1) < 0) { usleep(100000); }
    keyinput_disabled = true;
}

/**
 * @brief restart input event for other processes
 * 
 */
void keyinput_enable(void)
{
    if (!keyinput_disabled)
        return;
    while (ioctl(input_fd, EVIOCGRAB, 0) < 0) { usleep(100000); }
    keyinput_disabled = false;
}

#endif // KEYMON_INPUT_FD_H__
