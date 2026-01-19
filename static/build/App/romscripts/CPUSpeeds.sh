#!/bin/sh
scriptlabel="DynamicLabel"
sysdir=/mnt/SDCARD/.tmp_update

if [ "$3" = "DynamicLabel" ]; then
    emulabel="$4"
    should_hide=0
    case "$emulabel" in
        "NDS"|"ScummVM"|"Pico-8"|"Ports"|"GNGEO"|"PCSX-ReARMed"|"advancemame"|"ONScripter"|"PICO-8 standalone"|"Ports Collection")
            should_hide=1
            ;;
    esac
    if [ "$should_hide" -eq 1 ]; then
        DynamicLabel="none"
    else
        DynamicLabel="CPU Speeds"
    fi
    echo -n "$DynamicLabel" > /tmp/DynamicLabel.tmp
    exit 0
fi

if [ "$DynamicLabel" != "none" ]; then
    ROM_PATH="$1"
    EMU_PATH="$2"
    if [ ! -f "$ROM_PATH" ] && [ ! -d "$ROM_PATH" ]; then
        exit 0
    fi
    if [ ! -d "$EMU_PATH" ]; then
        exit 0
    fi
    romname=$(basename "$ROM_PATH" | sed 's/\.[^.]*$//')
    romcfgpath="$(dirname "$ROM_PATH")/.game_config/$romname.cfg"
    if [ -f "$romcfgpath" ]; then
        CORE_NAME=$(grep "core = " "$romcfgpath" | cut -d'"' -f2 | sed 's/_libretro//' | sed 's/^[[:space:]]*//')
    else
        CORE_NAME=""
    fi
    if [ -z "$CORE_NAME" ]; then
        launch_script=""
        if [ -f "$EMU_PATH/launch.sh" ]; then
            launch_script="$EMU_PATH/launch.sh"
        fi
        if [ -z "$launch_script" ] || [ ! -f "$launch_script" ]; then
            :
        else
            CORE_NAME=$(grep "^default_core=" "$launch_script" | cut -d'=' -f2 | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
            if [ -z "$CORE_NAME" ]; then
                core_path=$(grep "\-L" "$launch_script" | grep -o "\.retroarch/cores/[^ ]*\.so" | head -n1)
                if [ -n "$core_path" ]; then
                    CORE_NAME=$(basename "$core_path" | sed 's/_libretro\.so//' | sed 's/^[[:space:]]*//')
                fi
            fi
        fi
    fi
    if [ -z "$CORE_NAME" ]; then
        if [ -f "$launch_script" ]; then
            var_core=$(grep "\-L" "$launch_script" | grep -o "\${[^}]*}_libretro.so" | head -n1)
            if [ -n "$var_core" ]; then
                var_name=$(echo "$var_core" | sed 's/\${\([^}]*\)}.*/\1/')
                var_value=$(grep "^$var_name=" "$launch_script" | cut -d'=' -f2 | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
                if [ -n "$var_value" ]; then
                    CORE_NAME="$var_value"
                fi
            fi
            if [ -z "$CORE_NAME" ]; then
                core_path=$(grep "\-L" "$launch_script" | grep -o "\.retroarch/cores/[^ ]*\.so" | head -n1)
                if [ -n "$core_path" ]; then
                    CORE_NAME=$(basename "$core_path" | sed 's/_libretro\.so//' | sed 's/^[[:space:]]*//')
                fi
            fi
        fi
    fi
    if [ -z "$CORE_NAME" ]; then
        :
        exit 0
    fi
    info_file="/mnt/SDCARD/RetroArch/.retroarch/cores/${CORE_NAME}_libretro.info"
    if [ -f "$info_file" ]; then
        CORE_NAME=$(grep "^corename" "$info_file" | cut -d'=' -f2 | sed 's/^[[:space:]]*//;s/[[:space:]]*$//;s/^"//;s/"$//')
    fi
    config_dir=""
    for format in "$CORE_NAME" "$(echo "$CORE_NAME" | sed 's/\(.\)\(.*\)/\U\1\L\2/')" "$(echo "$CORE_NAME" | tr '[:lower:]' '[:upper:]')"; do
        if [ -n "$format" ] && [ -d "/mnt/SDCARD/Saves/CurrentProfile/config/$format" ]; then
            config_dir="/mnt/SDCARD/Saves/CurrentProfile/config/$format"
            break
        fi
    done
    if [ -z "$config_dir" ]; then
        config_dir="/mnt/SDCARD/Saves/CurrentProfile/config/$CORE_NAME"
        mkdir -p "$config_dir"
    fi
    cd /mnt/SDCARD/App/romscripts/ && ./cpuspeeds "$config_dir"
fi