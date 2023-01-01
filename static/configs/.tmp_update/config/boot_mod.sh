#!/bin/sh
# BOOT="gpio output 85 1; bootlogo 0 0 0 0 0; mw 1f001cc0 11; gpio out 8 0; sf probe 0;sf read 0x22000000 \${sf_kernel_start} \${sf_kernel_size}; gpio out 8 1; gpio output 4 1; bootm 0x22000000"
# echo --- before ---
# /etc/fw_printenv bootcmd
# /etc/fw_setenv bootcmd $BOOT
# echo --- after ---
# /etc/fw_printenv bootcmd
