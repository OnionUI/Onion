#!/bin/sh

echo ":: ADDING HIDDEN COLUMN TO PLAY ACTIVITY DATABASE"

sqlite3_binary="/mnt/SDCARD/.tmp_update/bin/sqlite3"
database_file="/mnt/SDCARD/Saves/CurrentProfile/play_activity/play_activity_db.sqlite"

if ! ${sqlite3_binary} ${database_file} ".schema rom" | grep -q "deletion_id"; then

    ${sqlite3_binary} ${database_file} <<EOF
    PRAGMA foreign_keys=off;
    BEGIN TRANSACTION;
    ALTER TABLE rom ADD COLUMN deletion_id INTEGER DEFAULT 0;
    COMMIT;
EOF
fi
