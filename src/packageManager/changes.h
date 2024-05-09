#ifndef PACMAN_CHANGES_H__
#define PACMAN_CHANGES_H__

#include "./globals.h"

int changesInstalls(void)
{
    int total = 0;
    for (int i = 0; i < tab_count; i++)
        total += changes_installs[i];
    return total;
}

int changesRemovals(void)
{
    int total = 0;
    for (int i = 0; i < tab_count; i++)
        total += changes_removals[i];
    return total;
}

int changesTotal(void) { return changesInstalls() + changesRemovals(); }

int totalInstalls(void)
{
    int total = 0;
    for (int i = 0; i < tab_count; i++)
        total += package_installed_count[i];
    return total;
}

/**
 * @brief Sets the "changed" value for each item and updates counts for a layer
 * 
 * @param mode 0: all off, 1: all on, 2: auto
 * @param layer 
 */
void setItemsInstallValue(int mode, int layer)
{
    for (int i = 0; i < package_count[layer]; i++) {
        Package *package = &packages[layer][i];
        bool is_active = mode == 2 ? package->has_roms : mode;
        bool new_value = is_active != package->installed;

        if (package->changed != new_value) {
            package->changed = new_value;

            if (package->installed) {
                changes_removals[layer] += new_value ? 1 : -1;
                if (!package->complete)
                    changes_installs[layer] += new_value ? -1 : 1;
            }
            else
                changes_installs[layer] += new_value ? 1 : -1;
        }
    }
}

#endif // PACMAN_CHANGES_H__