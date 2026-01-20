#!/bin/sh
# Script to migrate mGBA save files to gpSP save files
save_dir="/mnt/SDCARD/Saves/CurrentProfile/saves"
state_dir="/mnt/SDCARD/Saves/CurrentProfile/states"

skip_overwrite=0
if [ "$1" = "--skip-overwrite" ]; then
    skip_overwrite=1
    shift
fi

is_empty_save() {
    [ ! -f "$1" ] && return 0
    if od -An -vtx1 "$1" | grep -qv -E '^( 00)+$|^( ff)+$'; then
        return 1
    else
        return 0
    fi
}

transfer_mgba_save() {
    romname="$1"
    
    mgba_save="$save_dir/mGBA/$romname.srm"
    gpsp_save="$save_dir/gpSP/$romname.srm"
    gpsp_state="$state_dir/gpSP/$romname.state.auto"

    if [ ! -f "$mgba_save" ]; then
        echo "No mGBA save found for transfer."
        return 2  # Failure: Missing mGBA save
    fi

    if [ -f "$gpsp_save" ]; then
        # Check if the existing gpSP save is empty
        if is_empty_save "$gpsp_save"; then
            echo "Existing gpSP save is empty. Overwriting it with mGBA save."
            if [ -f "$gpsp_state" ]; then
                echo "Deleting incompatible gpSP save state: $gpsp_state"
                rm -f "$gpsp_state"  # Remove the save state due to inconsistency
            fi
        elif [ "$skip_overwrite" -eq 1 ]; then
            echo "Skip-overwrite option set. Skipping transfer to avoid overwriting non-empty gpSP save."
            return 1 # Skipped due to skip-overwrite option
        else
            # Prompt the user to confirm overwriting the existing save
            LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so /mnt/SDCARD/.tmp_update/bin/prompt \
                -t "Overwrite save?" \
                -m "A gpSP save already exists for this game.\n\
Do you want to overwrite it with the mGBA save?\n\
This will also delete the auto save state." \
                "Yes, overwrite save" \
                "No, keep existing save"
            prompt_result=$?
            if [ "$prompt_result" -ne 0 ]; then
                echo "User chose to keep the existing save. Transfer skipped."
                return 1  # User skipped transfer
            fi

            # Backup gpSP save before overwriting, with a timestamp
            timestamp=$(date +%Y%m%d%H%M%S)
            echo "Renaming existing gpSP save before overwriting..."
            mv "$gpsp_save" "${gpsp_save}.bak_${timestamp}"
            if [ -f "$gpsp_state" ]; then
                echo "Renaming existing gpSP auto save state."
                mv "$gpsp_state" "${gpsp_state}.bak_${timestamp}"
            fi
        fi
    fi

    # Perform the transfer (copy operation)
    echo "Transferring mGBA save to gpSP for $romname"
    mkdir -p "$(dirname "$gpsp_save")"
    if ! cp "$mgba_save" "$gpsp_save"; then
        echo "Error: Failed to copy mGBA save to gpSP directory."
        return 2  # Failure: Copy operation failed
    fi

    echo "Transfer complete for $romname."
    return 0  # Success
}

transfer_mgba_save "$1"
exit $?
