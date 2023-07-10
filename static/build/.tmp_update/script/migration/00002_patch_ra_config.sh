#!/bin/sh

version() {
    echo "$@" | awk -F. '{ f=$1; if (substr(f,2,1) == "v") f = substr(f,3); printf("%d%03d%03d%03d\n", f,$2,$3,$4); }'
}

prev_version=$(version "$(cat $sysdir/onionVersion/previous_version.txt)")

if [ $prev_version -lt $(version "4.1.0") ]; then
    cat > /tmp/onion_ra_patch.cfg <<- EOM
content_show_netplay = "true"
fastforward_frameskip = "false"
input_overlay_opacity = "1.000000"
input_toggle_fullscreen_btn = "3"
log_dir = "/mnt/SDCARD/.tmp_update/logs"
menu_show_overlays = "true"
menu_ticker_speed = "4.800000"
notification_show_autoconfig = "false"
notification_show_cheats_applied = "false"
notification_show_config_override_load = "false"
notification_show_patch_applied = "false"
notification_show_remap_load = "false"
notification_show_screenshot = "false"
notification_show_set_initial_disk = "false"
rgui_menu_color_theme = "0"
rgui_menu_theme_preset = ":/.retroarch/assets/rgui/Onion.cfg"
savestate_thumbnail_enable = "true"
settings_show_achievements = "true"
EOM

    # Patch RA config
    ./script/patch_ra_cfg.sh /tmp/onion_ra_patch.cfg

    rm /tmp/onion_ra_patch.cfg
else
    echo "Skipped"
fi
