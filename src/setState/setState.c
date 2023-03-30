#include <stdio.h>
#include <stdlib.h>

#include "system/state.h"
#include "utils/apps.h"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: setState [N]\nN: 0 - main menu, 1 - recents, 2 - "
               "favorites, 3 - games, 4 - expert, 5 - apps\n");
        return 1;
    }

    MainUIState state = atoi(argv[1]);
    int currpos = 0, total = 10;

    if (argc == 3)
        getAppPosition(argv[2], &currpos, &total);

    write_mainui_state(state, currpos, total);

    return 0;
}
