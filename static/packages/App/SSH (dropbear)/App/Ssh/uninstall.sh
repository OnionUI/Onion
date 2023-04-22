#!/bin/sh

pkill -9 gesftpserver
pkill -9 dropbear
rm -f /appconfigs/dropbear_dss_host_key
rm -f /appconfigs/dropbear_rsa_host_key
rm -f /appconfigs/dropbear_ecdsa_host_key
rm -f /appconfigs/dropbear_ed25519_host_key

