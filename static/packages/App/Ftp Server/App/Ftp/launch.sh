#!/bin/sh
echo $0 $*
cd $(dirname "$0")
pkill -9 bftpd
./bftpd -d
