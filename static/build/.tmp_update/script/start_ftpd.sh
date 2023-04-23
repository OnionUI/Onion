#!/bin/sh

pkill -9 tcpsvd
pkill -9 ftpd

tcpsvd -E 0.0.0.0 21 ftpd -w /mnt/SDCARD > /dev/null 2>&1 &

