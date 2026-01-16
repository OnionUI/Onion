#!/bin/sh
sysdir=/mnt/SDCARD/.tmp_update

logfile=$(basename "$0" .sh)
. $sysdir/script/log.sh

handle_compressed_roms() {


  log "::Handle zip files"

  if [ $# -ne 1 ]; then
    log "You should only pass the rom path\n"
    exit 1
  fi

  temp_folder=$sysdir/.tmp/
  mkdir -p "$temp_folder"

  rompath="$1"
  romext=$(echo "$(basename "$rompath")" | awk -F. '{print tolower($NF)}')
  rom_temp_folder="$(realpath "$(dirname "$rompath")")/.tmp/"
  mkdir -p "$rom_temp_folder"
  rom_real_path=$(realpath "$rompath")
  romname=""

  if [ "$romext" == "zip" ]; then
    romname=$(basename "$rompath" ".zip")
    if [ ! -f "$rom_temp_folder$romname".* ]; then
      unzip -o "$rom_real_path" -d "$temp_folder" > /dev/null
    fi
  else
    romname=$(basename "$rompath" ".7z")
    if [ ! -f "$rom_temp_folder$romname".* ]; then
      7z x "$rom_real_path" -o"$temp_folder" -y > /dev/null
    fi
  fi


  log "Processing files in $temp_folder \n"

  for f in "$temp_folder"*; do
    [ -f "$f" ] || continue  
    ext="${f##*.}"

    counter_file="$temp_folder.count_$ext"
    if [ ! -f "$counter_file" ]; then
      echo 0 > "$counter_file"
    fi
    count=$(cat "$counter_file")
    count=$((count + 1))
    echo "$count" > "$counter_file"

    if [ "$count" -gt 1 ]; then
      target="$rom_temp_folder${romname}${count}.$ext"
    else
      target="$rom_temp_folder${romname}.$ext"
    fi

    mv "$f" "$target"
    log "Moved $f -> $target \n"
  done

  rm -rf "$temp_folder"

  log "All files processed successfully.\n"
}
