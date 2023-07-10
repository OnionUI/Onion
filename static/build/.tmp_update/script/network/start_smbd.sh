export LD_PRELOAD=""
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/mnt/SDCARD/.tmp_update/lib/samba:/mnt/SDCARD/.tmp_update/lib/samba/private"
/mnt/SDCARD/.tmp_update/bin/samba/sbin/smbd --no-process-group -D
PASS=smbOnion
echo -ne "$PASS\n$PASS\n" | /mnt/SDCARD/.tmp_update/bin/samba/sbin/smbpasswd -s -a sduser
