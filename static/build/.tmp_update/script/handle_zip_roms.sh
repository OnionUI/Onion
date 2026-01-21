#!/bin/sh
sysdir=/mnt/SDCARD/.tmp_update

absolute_rom_path=""

handle_compressed_roms() {
  log "\n::Handle zip files"

  if [ $# -ne 1 ]; then
    echo "You should only pass the rom path"
    return 1
  fi

  rompath="$1"

  parent_folder="$(basename "$(dirname "$rompath")")"
  
  fz_file="$(realpath "$(dirname "$rompath")")"/.fz
  log "Rom path:$rompath"
  log "Parent folder path:$parent_folder"
  log "Fz File: $fz_file"

  if [ "$parent_folder" == "PS" ] || [ "$parent_folder" == "ARCADE" ];then
    absolute_rom_path="$rompath" # These emulators are multi-disc / Rom games and aren't compatible with the way we do this
    return 0
  fi

  if [ ! -f "$fz_file" ];then  # Checks if the user defined to use this script via the fz_file (Fast zip)
    absolute_rom_path="$rompath" # In the case of the emulators that handle everything correctly we can just pass to retroarch to handle it
    return 0
  fi

  temp_folder=$sysdir/.tmp/
  mkdir -p "$temp_folder"

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


  log "Processing files in $temp_folder"

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
    log "Moved $f -> $target"
  done

  rm -rf "$temp_folder"

  log "All files processed successfully."

  ret=$(find "$rom_temp_folder" -type f -name "$romname*" | head -1)
  absolute_rom_path="$ret"
}

