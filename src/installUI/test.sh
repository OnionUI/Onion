#!/bin/sh
installdir=`cd -- "$(dirname "$0")" >/dev/null 2>&1; pwd -P`
zipfile=$1

cleanup() {
    rm -rf \
        /tmp/.update_msg \
        $installdir/.installed \
        $installdir/.installFailed
}

unzip_progress() {
    zipfile="$1"
	msg="$2"
    dest="$3"
    total=`unzip -l "$zipfile" | tail -1 | grep -Eo "([0-9]+) files" | sed "s/[^0-9]*//g"`
    echo "   - Extract '$zipfile' ($total files) into $dest"

    unzip -ou "$zipfile" -d "$dest" | awk -v total="$total" -v out="/tmp/.update_msg" -v msg="$msg" 'BEGIN { cnt = 0; l = 0; printf "" > out; }{
        print $0;
        p = int(cnt * 100 / total);
        if (p != l) {
            printf "%s %3.0f%%\n", msg, p >> out;
            close(out);
            l = p;
        }
        cnt += 1;
    }'

    echo "$msg 100%" >> /tmp/.update_msg
}

main() {
    cleanup
    echo "Preparing update..." > /tmp/.update_msg

    ./installUI &
    sleep 1

    rm -rf $installdir/temp/
    mkdir -p temp

    if [ -f "$zipfile" ]; then
        unzip_progress "$zipfile" "1/2: Updating Onion..." temp
        sleep 5
    else
        echo "Invalid zip file"
        touch $installdir/.installFailed
        exit -1;
    fi

    echo "Update complete!" >> /tmp/.update_msg
    sleep 1

    touch $installdir/.waitConfirm
    sync
    
    echo "Press any button to turn off" >> /tmp/.update_msg
    sleep 1

    touch $installdir/.installed

    until [ ! -f $installdir/.waitConfirm ]; do
        sync
    done

    cleanup
}

main
