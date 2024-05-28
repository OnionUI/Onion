#!/bin/sh
mydir=$(dirname "$0")
cd $mydir

export HOME=$mydir
export PATH=$mydir:$PATH
export LD_LIBRARY_PATH=$mydir/libs:$LD_LIBRARY_PATH
export SDL_VIDEODRIVER=mmiyoo
export SDL_AUDIODRIVER=mmiyoo
export EGL_VIDEODRIVER=mmiyoo

. /mnt/SDCARD/.tmp_update/script/stop_audioserver.sh

CUST_LOGO=0
CUST_CPUCLOCK=1

# Uncomment this section if you need to customize the Drastic menu logo :
# if [ "$CUST_LOGO" == "1" ]; then
# echo "convert resources/logo/0.png to drastic_logo_0.raw"
# echo "convert resources/logo/1.png to drastic_logo_1.raw"
# ./png2raw
# fi

resolution=$(head -n 1 /tmp/screen_resolution)
if [ "$resolution" = "752x560" ]; then
    USE_752x560_RES=1
else
    USE_752x560_RES=0
fi

sv=$(cat /proc/sys/vm/swappiness)

echo 10 >/proc/sys/vm/swappiness # 60 by default

cd $mydir

if [ "$CUST_CPUCLOCK" == "1" ]; then
    cpuclock 1500
fi

l_triggers_swapped=$(jq ".swap_l1l2" "/mnt/SDCARD/Emu/NDS/resources/settings.json")

if [ $l_triggers_swapped -eq 1 ]; then
    touch /tmp/drastic_swap_l1l2
else
    rm -f /tmp/drastic_swap_l1l2
fi

./drastic "$1"

sync

echo $sv >/proc/sys/vm/swappiness

if [ "$USE_752x560_RES" == "1" ]; then
    fbset -g 640 480 640 960 32
fi
