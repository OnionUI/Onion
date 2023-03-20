#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/log.h"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: detectKey [KEY] - Example (MENU button): detectKey 1\n");
        return 1;
    }

    int key = atoi(argv[1]);

    FILE *kbd = fopen("/dev/input/event0", "r");

    char key_map[KEY_MAX / 8 + 1];
    memset(key_map, 0, sizeof(key_map));

    //  Fill the keymap with the current keyboard state
    ioctl(fileno(kbd), EVIOCGKEY(sizeof(key_map)), key_map);

    //  The key we want (and the seven others around it)
    int keyb = key_map[key / 8];
    //  Put a one in the same column as our key state will be in;
    int mask = 1 << (key % 8);

    return !(keyb & mask); //  Returns true if pressed otherwise false
}
