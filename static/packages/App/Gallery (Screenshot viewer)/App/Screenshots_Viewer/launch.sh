#!/bin/sh
echo ":: Gallery - Screenshots Viewer ::"

/mnt/SDCARD/.tmp_update/bin/infoPanel -d /mnt/SDCARD/Screenshots --show-theme-controls
ec=$?

# cancel or success from infoPanel
if [ $ec -eq 255 ] || [ $ec -eq 0 ] ; then
    exit 0
fi

/mnt/SDCARD/.tmp_update/bin/infoPanel -t Gallery -m "No screenshots found"
