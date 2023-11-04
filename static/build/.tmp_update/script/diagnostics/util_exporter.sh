# Add # Ignore for the file to be ignored (If it's a util only such as export)

# IGNORE

# If you add scripts that will generate logs, this script will pull anything from $sysdir/logdump so put them in that dir (it's removed after the archive is created)

sysdir="/mnt/SDCARD/.tmp_update"
workingdir="$sysdir/logdump"
runtime_logs_dir="$workingdir/logs"
filename="/mnt/SDCARD/log_export.7z"

## Source global utils
logfile=util_exporter
. $sysdir/script/log.sh
program=$(basename "$0" .sh)

move_runtime_logs() {
    log "Moving runtime logs"
    mkdir -p "$runtime_logs_dir"
    if [ -n "$(ls -A $sysdir/logs/)" ]; then
        cp -r /mnt/SDCARD/.tmp_update/logs/* "$runtime_logs_dir/"
    else
        log "No logs found in .tmp_update"
    fi
}

exporter() {
    log "Exporting and archiving logs"
    move_runtime_logs
    sync
    sleep 1
    
    7z a -y "$filename" "$workingdir/*" >/dev/null 2>&1

    if [ -s "$filename" ]; then
        log "Exported to: $filename"
        rm -rf $workingdir
    else
        log "Failed to export"
    fi
    
    exit
}

exporter