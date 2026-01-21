#!/bin/sh
# Script to rename gpSP save files from .sav to .srm and migrate mGBA saves to gpSP
save_dir=/mnt/SDCARD/Saves/CurrentProfile/saves
gpsp_dir="$save_dir/gpSP"
mgba_dir="$save_dir/mGBA"
migration_log="$gpsp_dir/migration_status.log"  # Centralized tracking file

# Create gpSP save directory if it doesn't exist
if [ ! -d "$gpsp_dir" ]; then
    if [ -e "$gpsp_dir" ]; then
        rm -f "$gpsp_dir" # Remove file if it exists (edge case)
    fi
    mkdir -p "$gpsp_dir"
fi

# Ensure the migration log file exists
touch "$migration_log"

# Helper: Mark a migration as needing manual transfer in the log
mark_for_manual_transfer() {
    romname="$1"
    echo "$romname" >> "$migration_log"
}

# Rename gpSP .sav files to .srm if they exist
for file in $gpsp_dir/*.sav; do
    new_file="${file%.sav}.srm"
    if [ ! -f "$new_file" ]; then
        echo "Renaming: $file -> $new_file"
        mv -n -- "$file" "$new_file"
    fi
done

# Check for mGBA saves and migrate them if appropriate
for mgba_save in "$mgba_dir/"*.srm; do
    [ -e "$mgba_save" ] || continue
    romname="${mgba_save##*/}" # Extract filename
    romname="${romname%.srm}" # Remove extension

    # Call the transfer script in skip-overwrite mode
    /mnt/SDCARD/.tmp_update/script/transfer_mgba_save.sh --skip-overwrite "$romname"
    result=$?
    if [ "$result" -eq 1 ]; then
        # If transfer skipped (e.g., due to existing non-empty gpSP save), mark for manual transfer
        # This ensures the user is notified next time they launch the game
        mark_for_manual_transfer "$romname"
    fi
done

# Remove old flag
if [ -f "$mgba_dir/.gpspImportDone" ]; then
    rm -f "$mgba_dir/.gpspImportDone"
fi

echo "Migration of gpSP saves and mGBA saves completed."
