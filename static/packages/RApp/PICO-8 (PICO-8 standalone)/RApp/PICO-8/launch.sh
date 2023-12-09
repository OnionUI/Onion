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



purge_devil() {
    if pgrep -f "/dev/l" > /dev/null; then
        echo "Process /dev/l is running. Killing it now..."
        killall -9 l
    else
        echo "Process /dev/l is not running."
    fi
}

# some users have reported black screens at boot. we'll check if the file exists, then check the keys to see if they match the known good config
fixconfig() {
    config_file="${picoconfig}.lexaloffle/pico-8/config.txt"
	echo "======================================== config_file $config_file" 

    default_video_settings="window_size 640 480\nscreen_size 640 480\nshow_fps 0\ntransform_screen 134"
    default_window_settings="windowed 0\nwindow_position -1 -1\nframeless 1\nfullscreen_method 2\nblit_method 0"

    if [ ! -f "$config_file" ]; then
        echo "Config file not found, creating with default values."
        printf "// :: Video Settings\n%s\n\n// :: Window Settings\n%s\n" "$default_video_settings" "$default_window_settings" > "$config_file"
        return
    fi

    echo "Config checker: Validating display settings in config.txt"

    for setting in window_size screen_size windowed window_position frameless fullscreen_method blit_method transform_screen; do
        current_value=$(grep "$setting" "$config_file")

        if [ -z "$current_value" ]; then
            case $setting in
                window_size|screen_size) printf "%s 640 480\n" "$setting" >> "$config_file" ;;
                windowed) printf "%s 0\n" "$setting" >> "$config_file" ;;
                window_position) printf "%s -1 -1\n" "$setting" >> "$config_file" ;;
                frameless) printf "%s 1\n" "$setting" >> "$config_file" ;;
                fullscreen_method) printf "%s 2\n" "$setting" >> "$config_file" ;;
                blit_method) printf "%s 0\n" "$setting" >> "$config_file" ;;
                transform_screen) printf "%s 134\n" "$setting" >> "$config_file" ;;
            esac
            echo "Added missing setting: ${setting}"
        else
            echo "Current ${setting} setting: $current_value"
            case $setting in
                window_size|screen_size)
                    sed -i "s/$setting 0 0/$setting 640 480/g" "$config_file" ;;
                transform_screen)
                    sed -i "s/$setting [0-9]+/$setting 134/g" "$config_file" ;;
            esac
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
	
    purge_devil
    fixconfig
    . /mnt/SDCARD/.tmp_update/script/stop_audioserver.sh
    libpadspblocker
	if [ "$filename" = "~Run PICO-8.pico-8" ]; then 
		pico8_dyn -splore
	else
		pico8_dyn -run "$rompath"
	fi
}

main() {
    echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
    start_pico
}

main
