#!/bin/sh

sqlite3_binary="/mnt/SDCARD/.tmp_update/bin/sqlite3"
database_file="/mnt/SDCARD/Saves/CurrentProfile/play_activity/play_activity_db.sqlite"

${sqlite3_binary} ${database_file} <<EOF
UPDATE rom SET deletion_id = 0;
EOF

return 0
