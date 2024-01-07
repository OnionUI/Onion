#!/bin/sh
echo $0 $*
picodir=$(dirname "$0")

export picodir="$picodir"
export picoconfig="/mnt/SDCARD/Saves/CurrentProfile/saves/PICO-8"
export rompath="$1"
export filename=$(basename "$rompath")

cd $picodir
export PATH=$PATH:$PWD/bin
export HOME=$picoconfig
export BBS_DIR="/mnt/SDCARD/Roms/PICO/splore/" # change me to a location for the bbs carts to go to

if [ ! -d "$BBS_DIR" ]; then
    mkdir -p "$BBS_DIR"
fi

# some users have reported black screens at boot. we'll check if the file exists, then check the keys to see if they match the known good config
fixconfig() {
    config_file="${picoconfig}/.lexaloffle/pico-8/config.txt"

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
            echo "$new_value" >>"$config_file"
            echo "Added missing setting: $setting"
        fi
    done

    echo "Updated settings:"
    grep -E "window_size|screen_size|windowed|window_position|frameless|fullscreen_method|blit_method|transform_screen" "$config_file"
}

start_pico() {

    if [ ! -e "$picodir/bin/pico8_dyn" ]; then
        infoPanel --title "PICO-8 binaries not found" --message "Native PICO-8 engine requires to purchase official \nbinaries which are not provided in Onion. \nGo to Lexaloffle's website, get Raspberry Pi version\n and copy \"pico8_dyn\" and \"pico8.dat\"\n to your SD card in \"/RApp/PICO-8/bin\"."
        cd /mnt/SDCARD/.tmp_update/script
        ./remove_last_recent_entry.sh
        exit
    fi

    export LD_LIBRARY_PATH="$picodir/lib:$LD_LIBRARY_PATH"
    export SDL_VIDEODRIVER=mmiyoo
    export SDL_AUDIODRIVER=mmiyoo
    export EGL_VIDEODRIVER=mmiyoo

    fixconfig

    . /mnt/SDCARD/.tmp_update/script/stop_audioserver.sh

    if [ "$filename" = "~Run PICO-8 with Splore.pico-8" ]; then
        num_files_before=$(ls -1 "$BBS_DIR" | wc -l)
        LD_PRELOAD="$picodir/lib/libcpt_hook.so" pico8_dyn -splore -preblit_scale 3 -pixel_perfect 0
        num_files_after=$(ls -1 "$BBS_DIR" | wc -l)
        if [ "$num_files_before" -ne "$num_files_after" ]; then
            rm -f /mnt/SDCARD/Roms/PICO/PICO_cache6.db
        fi

    else
        LD_PRELOAD="$picodir/lib/libcpt_hook.so" pico8_dyn -preblit_scale 3 -pixel_perfect 0 -run "$rompath"
    fi
}

main() {
    echo performance >/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
    start_pico
}

main
