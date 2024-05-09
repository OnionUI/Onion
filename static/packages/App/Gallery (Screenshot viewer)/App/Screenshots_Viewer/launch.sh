#!/bin/sh
echo ":: Gallery - Screenshots Viewer ::"

infoPanel -d /mnt/SDCARD/Screenshots --show-theme-controls
ec=$?

# cancel or success from infoPanel
if [ $ec -eq 255 ] || [ $ec -eq 0 ]; then
    exit 0
elif [ $ec -eq 1 ]; then
    infoPanel -t Gallery -m "No screenshots found"
else
    # something went wrong
    infoPanel -t Gallery -m "An error occurred - code: $ec"
fi
