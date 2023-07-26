import std/db_sqlite
import os
import strutils
import re

var directories: seq[string] = @[]


for dir in walkDir("Roms", relative = false):
    if os.dirExists(dir.path):
        directories.add(dir.path)

for dir in directories:
    let parts = dir.split("/")
    let lastPart = parts[^1]
    let filename = "/" & lastPart & "_cache6.db"
    let fullPath = dir & filename
    let db = open(fullPath, "", "", "")
    let tableName = lastPart & "_roms"
    echo "Working on " & tableName
    try:
        for x in db.fastRows(sql"SELECT * FROM ?", tableName):
            let name =  x[1].replace(re"\(.*?\)", "")
            echo "old name: " % x[1] & ", new name: " & name
            let id = x[0]
            db.exec(sql"UPDATE ? SET disp = ?, pinyin = ?, cpinyin = ? WHERE id = ?", tableName, name, name, name, id)
    except DbError:
        echo "Table not found"
    db.close()
