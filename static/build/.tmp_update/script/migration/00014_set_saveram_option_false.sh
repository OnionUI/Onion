#!/bin/sh

cat > /tmp/onion_ra_patch.cfg <<- EOM
block_sram_overwrite = "false"
EOM

# Patch RA config
./script/patch_ra_cfg.sh /tmp/onion_ra_patch.cfg

rm /tmp/onion_ra_patch.cfg
