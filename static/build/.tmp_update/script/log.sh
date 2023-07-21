program=$logfile
log() {
    if [ -f $sysdir/config/.logging ]; then
        echo -e "($program) $(date +"%Y-%m-%d %H:%M:%S"):" $* | tee -a "$sysdir/logs/$logfile.log"
    fi
}