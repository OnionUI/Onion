#!/bin/sh

# Enable network commands in RetroArch

RA_CFG=/mnt/SDCARD/RetroArch/.retroarch/retroarch.cfg
CMD_SETTING="network_cmd_enable"

if ! grep -qF "$CMD_SETTING" $RA_CFG; then                               # if the setting is not present (common case)
    echo "$CMD_SETTING = \"true\"" >>$RA_CFG                             # append the setting to the end of the file
elif grep -qF "$CMD_SETTING = \"false\"" $RA_CFG; then                   # else if the setting is set to false (unlikely but possible)
    sed -i "s/$CMD_SETTING = \"false\"/$CMD_SETTING = \"true\"/" $RA_CFG # change the setting to true
fi
