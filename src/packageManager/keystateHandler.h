#ifndef PACMAN_KEYSTATE_HANDLER_H__
#define PACMAN_KEYSTATE_HANDLER_H__

#include "system/keymap_sw.h"
#include "utils/keystate.h"

#include "./changes.h"
#include "./globals.h"
#include "./listActions.h"
#include "./render.h"
#include "./summary.h"

static KeyState keystate[320] = {RELEASED};

bool keystateHandler(bool *quit_flag, bool *apply_changes, bool show_confirm, bool auto_update)
{
    if (!updateKeystate(keystate, quit_flag, true, NULL)) {
        return false;
    }

    bool state_changed = false;

    if (keystate[SW_BTN_R1] >= PRESSED) {
        if (nTab < tab_count - 1)
            nTab++;
        else
            nTab = 0;
        nSelection = 0;
        nListPosition = 0;
        state_changed = true;
    }
    if (keystate[SW_BTN_L1] >= PRESSED) {
        if (nTab > 0)
            nTab--;
        else
            nTab = tab_count - 1;
        nSelection = 0;
        nListPosition = 0;
        state_changed = true;
    }

    if (nTab == summary_tab && surfaceSummary != NULL) {
        if (keystate[SW_BTN_R2] >= PRESSED) {
            if (summaryScrollBy(323))
                state_changed = true;
        }

        if (keystate[SW_BTN_L2] >= PRESSED) {
            if (summaryScrollBy(-323))
                state_changed = true;
        }

        if (keystate[SW_BTN_DOWN] >= PRESSED) {
            if (summaryScrollBy(scrollSpeed))
                state_changed = true;
        }

        if (keystate[SW_BTN_UP] >= PRESSED) {
            if (summaryScrollBy(-scrollSpeed))
                state_changed = true;
        }
    }
    else if (package_count[nTab] > 0) {
        if (keystate[SW_BTN_R2] >= PRESSED) {
            if ((nListPosition + 14) < package_count[nTab]) {
                nListPosition += 7;
            }
            else if ((nListPosition + 7) < package_count[nTab]) {
                nListPosition = package_count[nTab] - 7;
                nSelection = 6;
            }
            state_changed = true;
        }

        if (keystate[SW_BTN_L2] >= PRESSED) {
            if ((nListPosition - 7) > 0) {
                nListPosition -= 7;
            }
            else {
                nListPosition = 0;
                nSelection = 0;
            }
            state_changed = true;
        }

        if (keystate[SW_BTN_DOWN] >= PRESSED) {
            if (nSelection < 6) {
                nSelection++;
            }
            else if (nSelection + nListPosition <
                     package_count[nTab] - 1) {
                nListPosition++;
            }
            else if (keystate[SW_BTN_DOWN] == PRESSED &&
                     nSelection + nListPosition >=
                         package_count[nTab] - 1) {
                nSelection = nListPosition = 0;
            }
            state_changed = true;
        }

        if (keystate[SW_BTN_UP] >= PRESSED) {
            if (nSelection > 0) {
                nSelection--;
            }
            else if (nListPosition > 0) {
                nListPosition--;
            }
            else if (keystate[SW_BTN_UP] == PRESSED) {
                nSelection = 6;
                nListPosition = package_count[nTab] - 7;
            }
            state_changed = true;
        }
    }

    bool back_pressed = keystate[SW_BTN_B] == PRESSED;
    bool apply_pressed =
        keystate[SW_BTN_START] == PRESSED ||
        (keystate[SW_BTN_A] == PRESSED && nTab == summary_tab);

    if (back_pressed || apply_pressed) {
        if (apply_pressed && nTab != summary_tab) {
            nTab = summary_tab;
            nSelection = 0;
            nListPosition = 0;
        }
        else {
            if (apply_pressed)
                *apply_changes = true;

            if (show_confirm) {
                if (*apply_changes) {
                    if (changesTotal() > 0 ||
                        (auto_update &&
                         package_installed_count[0] > 0) ||
                        confirmDoNothing(keystate))
                        *quit_flag = true;
                }
                else if (confirmDoNothing(keystate))
                    *quit_flag = true;
            }
            else
                *quit_flag = true;
        }
        state_changed = true;
    }

    if (package_count[nTab] > 0) {
        if (keystate[SW_BTN_A] == PRESSED ||
            keystate[SW_BTN_LEFT] == PRESSED ||
            keystate[SW_BTN_RIGHT] == PRESSED) {
            int pos = nListPosition + nSelection;
            if (pos < package_count[nTab]) {
                Package *package = &packages[nTab][pos];
                bool prev_value = package->changed;

                if (keystate[SW_BTN_A] == PRESSED)
                    package->changed = !package->changed;
                else if (keystate[SW_BTN_LEFT] == PRESSED)
                    package->changed = package->installed;
                else if (keystate[SW_BTN_RIGHT] == PRESSED)
                    package->changed = !package->installed;

                if (package->changed != prev_value) {
                    if (package->installed) {
                        changes_removals[nTab] +=
                            package->changed ? 1 : -1;
                        if (!package->complete)
                            changes_installs[nTab] +=
                                package->changed ? -1 : 1;
                    }
                    else {
                        changes_installs[nTab] +=
                            package->changed ? 1 : -1;
                    }
                    refreshSummary();
                    state_changed = true;
                }
            }
        }

        if (keystate[SW_BTN_X] == PRESSED) {
            layerToggleAll(nTab, false);
            refreshSummary();
            state_changed = true;
        }

        if (keystate[SW_BTN_Y] == PRESSED) {
            layerReset(nTab);
            refreshSummary();
            state_changed = true;
        }
    }

    return state_changed;
}

#endif // PACMAN_KEYSTATE_HANDLER_H__