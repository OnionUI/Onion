#ifndef PACMAN_LIST_ACTIONS_H__
#define PACMAN_LIST_ACTIONS_H__

#include "./changes.h"
#include "./globals.h"

void layerToggleAll(int layer, bool only_when_all_off)
{
    bool all_on = true;
    bool all_off = true;
    bool has_roms = false;

    for (int i = 0; i < package_count[layer]; i++) {
        Package *package = &packages[layer][i];
        if (package->installed == package->changed) {
            all_on = false;
            break;
        }
    }
    for (int i = 0; i < package_count[layer]; i++) {
        Package *package = &packages[layer][i];
        if (package->installed != package->changed) {
            all_off = false;
            break;
        }
    }
    for (int i = 0; i < package_count[layer]; i++) {
        Package *package = &packages[layer][i];
        if (package->has_roms) {
            has_roms = true;
            break;
        }
    }

    int mode = 1;
    if (all_off && has_roms)
        mode = 2;
    else if (all_on)
        mode = 0;

    if (only_when_all_off && !all_off)
        return;

    setItemsInstallValue(mode, layer);
}

void layerReset(int layer)
{
    for (int i = 0; i < package_count[layer]; i++) {
        Package *package = &packages[layer][i];

        if (package->changed) {
            if (package->installed) {
                changes_removals[layer]--;
                if (!package->complete)
                    changes_installs[layer]++;
            }
            else
                changes_installs[layer]--;

            package->changed = false;
        }
    }
}

#endif // PACMAN_LIST_ACTIONS_H__
