#!/bin/bash

# Creates full_resolution files where appropriate
# If this file is present, the program will be launched in 560p by runtime.sh

# Include hidden folders
shopt -s dotglob

packages="./build/App/PackageManager/data"

# Emulators using RetroArch
for dir in "$packages"/{Emu,RApp}/*/{Emu,RApp}/*/; do
    if [ -f "${dir}launch.sh" ] && grep -qF "retroarch" "${dir}launch.sh"; then
        touch "${dir}full_resolution"
    fi
done

# DraStic
touch "$packages/Emu/Nintendo - DS (Drastic)/Emu/NDS/full_resolution"

# RetroArch shortcut
touch "$packages/App/RetroArch (Shortcut)/App/RetroArch/full_resolution"
