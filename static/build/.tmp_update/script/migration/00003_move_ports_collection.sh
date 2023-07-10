#!/bin/sh

if [ ! -d /mnt/SDCARD/Emu/PORTS/Binaries ] || [ ! -d /mnt/SDCARD/Roms/PORTS/Binaries ]; then
    echo "Skipped"
    exit 0
fi

if [ -d /mnt/SDCARD/Emu/PORTS/Binaries ]; then
    echo "Ports collection found! Moving..."
    mkdir -p /mnt/SDCARD/Roms/PORTS/Binaries
    mv -f /mnt/SDCARD/Emu/PORTS/Binaries/* /mnt/SDCARD/Roms/PORTS/Binaries
    mv -f /mnt/SDCARD/Emu/PORTS/PORTS/* /mnt/SDCARD/Roms/PORTS
    rmdir /mnt/SDCARD/Emu/PORTS/Binaries
    rmdir /mnt/SDCARD/Emu/PORTS/PORTS
    rm -f /mnt/SDCARD/Roms/PORTS/PORTS_cache2.db
    rm -f /mnt/SDCARD/Emu/PORTS/config.json # Triggers a reinstall
fi
if [ -d /mnt/SDCARD/Roms/PORTS/Binaries ]; then
    mv -f /mnt/SDCARD/Roms/PORTS /mnt/SDCARD/Roms/PORTS_old
fi
