# Credits: gotbletu (@gmail|twitter|youtube|github|lbry)
#          adapted by Onion Team (https://github.com/OnionUI/Onion)

count_m3u="/tmp/count_m3u"

# target files extension
EXT_INT="cue|gdi|chd|pbp|iso|dsk"

helpmsg() {
  echo "desc: create m3u playlist for multi disc games [$EXT_INT]"
  echo "    e.g Playstation, Sega CD/Mega CD, NeoGeo CD,"
  echo "    PC Engine CD/PC-FX, Amiga"
  echo ""
  echo "usage: ${0##*/} [options] [folder name]"
  echo ""
  echo "options:"
  echo "  -md  create multiple directories : one directory and one m3u playlist for each game"
  echo "      from"
  echo "          Novastorm (USA) (Disc 1).bin"
  echo "          Novastorm (USA) (Disc 1).cue"
  echo "          Novastorm (USA) (Disc 2).bin"
  echo "          Novastorm (USA) (Disc 2).cue"
  echo "          Metal Gear Solid (USA) (Disc 1) (Rev 1).chd"
  echo "          Metal Gear Solid (USA) (Disc 2) (Rev 1).chd"
  echo "      to"
  echo "          Novastorm (USA).m3u"
  echo "          Metal Gear Solid (USA) (Rev 1).m3u"
  echo "          /.Novastorm (USA)/Novastorm (USA) (Disc 1).bin"
  echo "          /.Novastorm (USA)/Novastorm (USA) (Disc 1).cue"
  echo "          /.Novastorm (USA)/Novastorm (USA) (Disc 2).bin"
  echo "          /.Novastorm (USA)/Novastorm (USA) (Disc 2).cue"
  echo "          /.Metal Gear Solid (USA) (Rev 1)/Metal Gear Solid (USA) (Disc 1) (Rev 1).chd"
  echo "          /.Metal Gear Solid (USA) (Rev 1)/Metal Gear Solid (USA) (Disc 2) (Rev 1).chd"
  echo ""
  echo "  -sd create a single \"multi-disc\" directory for all multidisk files"
  echo "      from"
  echo "          Heart of Darkness (USA) (Disc 1).bin"
  echo "          Heart of Darkness (USA) (Disc 1).cue"
  echo "          Heart of Darkness (USA) (Disc 2).bin"
  echo "          Heart of Darkness (USA) (Disc 2).cue"
  echo "          Lunar 2 - Eternal Blue Complete (USA) (Disc 1).chd"
  echo "          Lunar 2 - Eternal Blue Complete (USA) (Disc 2).chd"
  echo "          Lunar 2 - Eternal Blue Complete (USA) (Disc 3).chd"
  echo "      to"
  echo "          Heart of Darkness (USA).m3u"
  echo "          Lunar 2 - Eternal Blue Complete (USA).m3u"
  echo "          .multi-disc/Heart of Darkness (USA) (Disc 1).bin"
  echo "          .multi-disc/Heart of Darkness (USA) (Disc 1).cue"
  echo "          .multi-disc/Heart of Darkness (USA) (Disc 2).bin"
  echo "          .multi-disc/Heart of Darkness (USA) (Disc 2).cue"
  echo "          .multi-disc/Lunar 2 - Eternal Blue Complete (USA) (Disc 1).chd"
  echo "          .multi-disc/Lunar 2 - Eternal Blue Complete (USA) (Disc 2).chd"
  echo "          .multi-disc/Lunar 2 - Eternal Blue Complete (USA) (Disc 3).chd"
  echo ""
  echo "  -h, --help                 display this help message"
  echo ""
  echo "Swap Disc: Retroarch > [Load Your Game] > Quick Menu > Disc Control"
}

if [ "$1" = -h ] || [ "$1" = --help ]; then
  helpmsg
  exit 0
elif [ "$1" = -md ]; then
  TARGETFOLDER="-md"
elif [ "$1" = -sd ]; then
  TARGETFOLDER="-sd"
else # case for eventual GLO menu
  LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so prompt -t "Multi-disc - Target Directory" -m "Choose the sub folder name where\noriginal disc files will be stored." \
    "Single directory (\".multi-disc\")" \
    "Multiple directories (\".Game_Name\")"
  retcode=$?

  if [ $retcode -eq 0 ]; then
    TARGETFOLDER="-sd"
  elif [ $retcode -eq 1 ]; then
    TARGETFOLDER="-md"
  elif [ $retcode -eq 255 ]; then
    exit
  fi
fi

echo 0 >"$count_m3u"
# create missing cue files.
"/mnt/SDCARD/.tmp_update/script/cue_gen.sh"

if [ "$TARGETFOLDER" = -sd ]; then
  DIR_NAME=".multi-disc"
fi

DIR_LIST="PS SEGACD NEOCD PCE PCFX AMIGA"
for dir in $DIR_LIST; do
  full_path="/mnt/SDCARD/Roms/$dir"
  echo "============================= $full_path ============================="
  if [ -d "$full_path" ]; then
    cd "$full_path"
    ### TYPE 1: TitleOfGame (USA) (Disc 1).chd
    #---------------------------------------------------------------------------------
    find . -maxdepth 1 ! -iname '*.m3u' -type f -iname "*([Dd][Ii][Ss][KkCc] 1).*[$EXT_INT]" | while read line; do
      # searching for a "Disc 1" file.
      FILE_NAME="$(echo "${line%.*}" | sed 's@./@@g' | sed 's@ ([Dd][Ii][Ss][KkCc] 1)@@g')"
      if [ "$TARGETFOLDER" = -md ]; then
        DIR_NAME=".$(echo "${line%.*}" | sed 's@./@@g' | sed 's@ ([Dd][Ii][Ss][KkCc] 1)@@g')"
        DIR_NAME="${DIR_NAME%"${DIR_NAME##*[![:space:]]}"}" # remove spaces at the end
      fi
      SEARCH_NAME="$(echo "${line%.*}" | sed 's@./@@g' | sed 's@([Dd][Ii][Ss][KkCc] 1)@([Dd][Ii][Ss][KkCc] ?)@g')"
      mkdir -p "$DIR_NAME"
      echo -e "****************\nType 1: ${line}\n**** SEARCH_NAME : $SEARCH_NAME \n**** DIR_NAME : $DIR_NAME\n**** FILE_NAME : $FILE_NAME"
      count=$(cat "$count_m3u")
      count=$((count + 1))
      echo "$count" >"$count_m3u"
      sync
      # move matching files to directory
      find . -maxdepth 1 ! -iname '*.m3u' -type f -iname "$SEARCH_NAME.*" -exec mv -n -- '{}' "$DIR_NAME" \;
      # Create m3u playslist file
      find "$DIR_NAME" ! -iname '*.m3u' -type f -iname "$SEARCH_NAME*.*[$EXT_INT]" | sed -e 's/^//' | sort >"$FILE_NAME".m3u
    done

    ### TYPE 2: AnotherTitleOfGame (USA) (Disc 1) (Rev 2).chd
    #---------------------------------------------------------------------------------
    find . -maxdepth 1 ! -iname '*.m3u' -type f -iname "*([Dd][Ii][Ss][KkCc] 1) *.*[$EXT_INT]" | while read line; do
      # searching for a "Disc 1" file.
      FILE_NAME="$(echo "${line%.*}" | sed 's@./@@g' | sed 's@ ([Dd][Ii][Ss][KkCc] 1)@@g')"
      if [ "$TARGETFOLDER" = -md ]; then
        DIR_NAME=".$(echo "${line%.*}" | sed 's@./@@g' | sed 's@ ([Dd][Ii][Ss][KkCc] 1)@@g')"
        DIR_NAME="${DIR_NAME%"${DIR_NAME##*[![:space:]]}"}" # remove spaces at the end
      fi
      SEARCH_NAME="$(echo "${line%.*}" | sed 's@./@@g' | sed 's@([Dd][Ii][Ss][KkCc] 1)@([Dd][Ii][Ss][KkCc] ?)@g')"
      echo -e "****************\nType 2: ${line}\n**** SEARCH_NAME : $SEARCH_NAME \n**** DIR_NAME : $DIR_NAME\n**** FILE_NAME : $FILE_NAME"
      count=$(cat "$count_m3u")
      count=$((count + 1))
      echo "$count" >"$count_m3u"
      sync
      mkdir -p "$DIR_NAME"
      # move matching files to directory
      find . -maxdepth 1 ! -iname '*.m3u' -type f -iname "$SEARCH_NAME.*" -exec mv -n -- '{}' "$DIR_NAME" \;
      # Create m3u playslist file
      find "$DIR_NAME" ! -iname '*.m3u' -type f -iname "$SEARCH_NAME*.*[$EXT_INT]" | sed -e 's/^//' | sort >"$FILE_NAME".m3u
    done

    ### TYPE 3: AmstradMSXTitleOfGame (19xx)(Developer)(Disc 1 of 3).dsk
    #---------------------------------------------------------------------------------
    find . -maxdepth 1 ! -iname '*.m3u' -type f -iname "*([Dd][Ii][Ss][KkCc] 1 of ?).*[$EXT_INT]" | while read line; do
      # searching for a "Disc 1" file.
      FILE_NAME="$(echo "${line%.*}" | sed 's@./@@g' | sed 's@([Dd][Ii][Ss][KkCc] 1 of .*)@@g')"
      if [ "$TARGETFOLDER" = -md ]; then
        DIR_NAME=".$(echo "${line%.*}" | sed 's@./@@g' | sed 's@([Dd][Ii][Ss][KkCc] 1 of .*)@@g')"
        DIR_NAME="${DIR_NAME%"${DIR_NAME##*[![:space:]]}"}" # remove spaces at the end
      fi
      SEARCH_NAME="$(echo "${line%.*}" | sed 's@./@@g' | sed 's@([Dd][Ii][Ss][KkCc] 1 of@([Dd][Ii][Ss][KkCc] ? of@g')"
      # SEARCH_NAME="$(echo "${line%.*}" | sed 's@./@@g' | sed 's@([Dd][Ii][Ss][KkCc] 1)@([Dd][Ii][Ss][KkCc] ?)@g')"
      echo -e "****************\nType 3: ${line}\n**** SEARCH_NAME : $SEARCH_NAME \n**** DIR_NAME : $DIR_NAME\n**** FILE_NAME : $FILE_NAME"
      count=$(cat "$count_m3u")
      count=$((count + 1))
      echo "$count" >"$count_m3u"
      sync
      mkdir -p "$DIR_NAME"
      # move matching files to directory
      find . -maxdepth 1 ! -iname '*.m3u' -type f -iname "$SEARCH_NAME*.*" -exec mv -n -- '{}' "$DIR_NAME" \;
      # Create m3u playslist file
      find "$DIR_NAME" ! -iname '*.m3u' -type f -iname "$SEARCH_NAME*.*[$EXT_INT]" | sed -e 's/^//' | sort >"$FILE_NAME".m3u
    done
  else
    echo "Directory $full_path does not exist."
  fi
done

if [ "$#" -eq 0 ]; then # when launched by GLO menu, without args.
  infoPanel --title "Multi-disc Creator" --message "$(cat "$count_m3u") playlist files created." --auto
fi
