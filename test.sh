#!/bin/sh
cmd="$1"
mainui_target=/mnt/SDCARD/miyoo/app/MainUI
clean_flag=/mnt/SDCARD/miyoo/app/.isClean
expert_flag=/mnt/SDCARD/miyoo/app/.isExpert
is_device_model_changed=0

if [ ! -f /customer/app/axp_test ]; then        
    touch /tmp/deviceModel
    printf "283" > /tmp/deviceModel
    deviceModel=283
else
    touch /tmp/deviceModel
    printf "354" > /tmp/deviceModel
    deviceModel=354
fi

if [ "$cmd" == "clean" ]; then
    # Should be clean
    if [ ! -f $clean_flag ] || [ -f $expert_flag ] || [ $is_device_model_changed -eq 1 ]; then
        # Should be clean
        if [ ! -f $clean_flag ] || [ -f $expert_flag ] || [ $is_device_model_changed -eq 1 ] || [ ! -f $mainui_target ]; then
            rm -f $expert_flag 2>&1 > /dev/null
            echo "$sysdir/bin/MainUI-$deviceModel-clean" $mainui_target
            touch $clean_flag
        fi
    fi
elif [ "$cmd" == "expert" ]; then
    # Should be expert
    if [ ! -f $expert_flag ] || [ -f $clean_flag ] || [ $is_device_model_changed -eq 1 ] || [ ! -f $mainui_target ]; then
        rm -f $clean_flag 2>&1 > /dev/null
        echo "$sysdir/bin/MainUI-$deviceModel-expert" $mainui_target
        touch $expert_flag
    fi
fi
