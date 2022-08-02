#!/bin/sh
installdir=`cd -- "$(dirname "$0")" >/dev/null 2>&1; pwd -P`
zipfile=$1

cleanup() {
    rm -rf \
        $installdir/.update_msg \
        $installdir/.installed \
        $installdir/.installFailed
}

unzip_progress() {
    zipfile=$1
	msg=$2
    dest=$3
    total=`unzip -l "$zipfile" | tail -1 | grep -Eo "([0-9]+) files" | sed "s/[^0-9]*//g"`
    unzip -d "$dest" -o "$zipfile" | awk -v total="$total" -v out="$installdir/.update_msg" -v msg="$msg" 'BEGIN{cnt = 0; l = 0}{
        p = int(cnt * 100 / total);
        if (p != l) {
            printf "%s %3.0f%%\n", msg, p >> out;
            close(out);
            l = p;
        }
        cnt += 1
    }'
    echo "$msg 100%" >> $installdir/.update_msg
}

cleanup
echo "Preparing update..." > $installdir/.update_msg

./installUI &
sleep 1

rm -rf $installdir/temp/
mkdir -p temp

if [ -f "$zipfile" ]; then
    unzip_progress "$zipfile" "Updating core..." temp
else
    echo "Invalid zip file"
    touch $installdir/.installFailed
    exit -1;
fi

install_retroarch() {
	install_ra=1
	msg="$1"

	# An existing version of Onion's RetroArch exist
	if [ -f onion_ra_version.txt ] && [ -f ra_package_version.txt ]; then
		current_ra_version=`cat onion_ra_version.txt`
		new_ra_version=`cat ra_package_version.txt`

		# Install if packaged RA version is greater
		if [ $(version $current_ra_version) -ge $(version $new_ra_version) ]; then
			install_ra=0
		fi
	fi

	# Install RetroArch only if necessary
	if [ $install_ra -eq 1 ]; then
        echo "$msg 0%" > $installdir/.update_msg
        sleep 2
        echo "$msg 100%" > $installdir/.update_msg
        sleep 1
	fi
}

version() {
	echo "$@" | awk -F. '{ printf("%d%03d%03d%03d\n", $1,$2,$3,$4); }';
}

install_retroarch "Updating RetroArch..."

touch $installdir/.installed
sleep 1

cleanup
