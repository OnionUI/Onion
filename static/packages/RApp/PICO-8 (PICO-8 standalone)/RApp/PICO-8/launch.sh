#!/bin/sh
echo $0 $*
picodir=`dirname "$0"`

export picodir="$picodir"
export picoconfig="/mnt/SDCARD/Saves/CurrentProfile/saves/PICO-8/"
export rompath="$1"
export filename=`basename "$rompath"`

cd $picodir
export PATH=$PATH:$PWD/bin
export HOME=$picoconfig


# some users have reported black screens at boot. we'll check if the file exists, then check the keys to see if they match the known good config
fixconfig() {
    config_file="${picodir}/.lexaloffle/pico-8/config.txt"

    if [ ! -f "$config_file" ]; then
        echo "Config file not found, creating with default values."
        return
    fi

    echo "Config checker: Validating display settings in config.txt"

    set_window_size="window_size 640 480"
    set_screen_size="screen_size 640 480"
    set_windowed="windowed 0"
    set_window_position="window_position -1 -1"
    set_frameless="frameless 1"
    set_fullscreen_method="fullscreen_method 0"
    set_blit_method="blit_method 2"
    set_transform_screen="transform_screen 134"

    for setting in window_size screen_size windowed window_position frameless fullscreen_method blit_method transform_screen; do
        case $setting in
            window_size) new_value="$set_window_size" ;;
            screen_size) new_value="$set_screen_size" ;;
            windowed) new_value="$set_windowed" ;;
            window_position) new_value="$set_window_position" ;;
            frameless) new_value="$set_frameless" ;;
            fullscreen_method) new_value="$set_fullscreen_method" ;;
            blit_method) new_value="$set_blit_method" ;;
            transform_screen) new_value="$set_transform_screen" ;;
        esac

        if grep -q "^$setting" "$config_file"; then
            sed -i "s/^$setting.*/$new_value/" "$config_file"
            echo "Updated setting: $setting"
        else
            echo "$new_value" >> "$config_file"
            echo "Added missing setting: $setting"
        fi
    done

    echo "Updated settings:"
    grep -E "window_size|screen_size|windowed|window_position|frameless|fullscreen_method|blit_method|transform_screen" "$config_file"
}


# when wifi is restarted, udhcpc and wpa_supplicant may be started with libpadsp.so preloaded, this is bad as they can hold mi_ao open even after audioserver has been killed.
libpadspblocker() { 
    wpa_pid=$(ps -e | grep "[w]pa_supplicant" | awk 'NR==1{print $1}')
    udhcpc_pid=$(ps -e | grep "[u]dhcpc" | awk 'NR==1{print $1}')
    if [ -n "$wpa_pid" ] && [ -n "$udhcpc_pid" ]; then
        if grep -q "libpadsp.so" /proc/$wpa_pid/maps || grep -q "libpadsp.so" /proc/$udhcpc_pid/maps; then
            echo "Network Checker: $wpa_pid(WPA) and $udhcpc_pid(UDHCPC) found preloaded with libpadsp.so"
            unset LD_PRELOAD
            killall -9 wpa_supplicant
            killall -9 udhcpc 
            $miyoodir/app/wpa_supplicant -B -D nl80211 -iwlan0 -c /appconfigs/wpa_supplicant.conf & 
            udhcpc -i wlan0 -s /etc/init.d/udhcpc.script &
            echo "Network Checker: Removing libpadsp.so preload on wpa_supp/udhcpc"
        fi
    fi
}

start_pico() {

	if [ ! -e "$picodir/bin/pico8_dyn" ]; then
		infoPanel --title "PICO-8 binaries not found" --message "Native PICO-8 engine requires to purchase official \nbinaries which are not provided in Onion. \nGo to Lexaloffle's website, get Raspberry Pi version\n and copy \"pico8_dyn\" and \"pico8.dat\"\n to your SD card in \"/RApp/PICO-8/bin\"."
		exit
	fi
	
    export LD_LIBRARY_PATH="$picodir/lib:$LD_LIBRARY_PATH"
    export SDL_VIDEODRIVER=mmiyoo
    export SDL_AUDIODRIVER=mmiyoo
    export EGL_VIDEODRIVER=mmiyoo
	
    fixconfig
    . /mnt/SDCARD/.tmp_update/script/stop_audioserver.sh
    libpadspblocker
	if [ "$filename" = "~Run PICO-8.pico-8" ]; then 
		pico8_dyn -splore -preblit_scale 3 -pixel_perfect 0
	else
		pico8_dyn -preblit_scale 3 -pixel_perfect 0 -run "$rompath"
	fi
}

main() {
    echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
    start_pico
}

main
