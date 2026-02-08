#!/bin/sh

set -u

SDCARD="/mnt/SDCARD"
APP_ROOT="$SDCARD/App"
ROMS_ROOT="$SDCARD/Roms"
EMU_ROOT="$SDCARD/Emu"

KIDMODE_APP_NAME="Kids_Mode"
KIDMODE_APP_ALT_NAME="Kids Mode"
UNLOCKER_APP_NAME="Kids_Unlock"

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname "$0")" >/dev/null 2>&1 && pwd -P)"
UNLOCKER_TEMPLATE_DIR="$SCRIPT_DIR/unlocker"

KM_ROOT="$SDCARD/.KidMode"
KM_BACKUP_DIR="$KM_ROOT/backup"
KM_STAGE_DIR="$KM_ROOT/stage"
KM_STATE_FILE="$KM_ROOT/state.json"
KM_LOCKDIR="$KM_ROOT/op.lock"
KM_SENTINEL=".kidmode_kiosk"
KM_LOG_FILE="$KM_ROOT/kidmode.log"
MANAGER_TEMPLATE_DIR="$KM_ROOT/manager_template"

LEGACY_CONFIG_DIR="$SDCARD/Saves/CurrentProfile/kidmode"
LEGACY_CONFIG_FILE="$LEGACY_CONFIG_DIR/config.json"
CONFIG_DIR="$KM_ROOT"
CONFIG_FILE="$CONFIG_DIR/config.json"
PANIC_FILE="$SDCARD/reset_kidmode.txt"

PROMPT_BIN="$SDCARD/.tmp_update/bin/prompt"
PINPAD_BIN="$SDCARD/.tmp_update/bin/pinpad"
KBINPUT_BIN="$SDCARD/.tmp_update/bin/kbinput"
RESET_LIST_SH="$SDCARD/.tmp_update/script/reset_list.sh"
LD_PRELOAD_LIB="$SDCARD/miyoo/lib/libpadsp.so"

DEFAULT_ICON_OFF="../../Icons/Default/app/guest_off.png"
DEFAULT_ICON_ON="../../Icons/Default/app/guest_on.png"

RA_CONFIG_FILE="$SDCARD/RetroArch/.retroarch/retroarch.cfg"
RA_BACKUP_FILE="$KM_ROOT/retroarch.cfg.before_kidmode"

umask 022

usage() {
    cat <<USAGE
Usage:
  kidmodectl.sh manager
  kidmodectl.sh lock
  kidmodectl.sh unlock --pin 1234
  kidmodectl.sh unlock-interactive
  kidmodectl.sh recover
  kidmodectl.sh status
  kidmodectl.sh set-pin 1234
  kidmodectl.sh set-pin-interactive
  kidmodectl.sh toggle-hide-games
  kidmodectl.sh toggle-lock-ra-settings
  kidmodectl.sh manage-allowed-apps
USAGE
}

log_msg() {
    mkdir -p "$KM_ROOT"
    printf '%s %s\n' "$(date '+%Y-%m-%d %H:%M:%S')" "$*" >> "$KM_LOG_FILE"
}

need_command() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "kidmodectl: missing dependency: $1" >&2
        exit 1
    fi
}

state_write() {
    phase="$1"
    message="$2"
    mkdir -p "$KM_ROOT"
    tmp="/tmp/kidmode_state.$$"
    jq -n \
        --arg phase "$phase" \
        --arg message "$message" \
        --argjson updated_at "$(date +%s)" \
        '{phase:$phase, updated_at:$updated_at, message:$message}' > "$tmp"
    mv "$tmp" "$KM_STATE_FILE"
}

generate_salt() {
    if [ -r /dev/urandom ]; then
        dd if=/dev/urandom bs=8 count=1 2>/dev/null | od -An -tx1 | tr -d ' \n'
    else
        printf '%s' "$(date +%s)$$"
    fi
}

hash_pin() {
    pin="$1"
    salt="$2"
    if command -v sha256sum >/dev/null 2>&1; then
        printf '%s' "${salt}${pin}" | sha256sum | awk '{print $1}'
        return 0
    fi

    if command -v busybox >/dev/null 2>&1; then
        printf '%s' "${salt}${pin}" | busybox sha256sum | awk '{print $1}'
        return 0
    fi

    echo "kidmodectl: missing dependency: sha256sum" >&2
    return 1
}

is_valid_pin() {
    case "$1" in
        [0-9][0-9][0-9][0-9])
            return 0
            ;;
        *)
            return 1
            ;;
    esac
}

ensure_layout() {
    need_command jq

    mkdir -p "$KM_ROOT" "$KM_BACKUP_DIR" "$KM_STAGE_DIR" "$CONFIG_DIR"

    # Migrate old per-profile config to global config location.
    if [ ! -f "$CONFIG_FILE" ] && [ -f "$LEGACY_CONFIG_FILE" ]; then
        cp "$LEGACY_CONFIG_FILE" "$CONFIG_FILE"
        log_msg "Migrated legacy config from $LEGACY_CONFIG_FILE"
    fi

    if [ ! -f "$KM_STATE_FILE" ]; then
        state_write "idle" "Initialized state file."
    fi

    if [ ! -f "$CONFIG_FILE" ]; then
        salt="$(generate_salt)"
        hash="$(hash_pin "0000" "$salt" 2>/dev/null || true)"
        tmp="/tmp/kidmode_config.$$"
        if [ -n "$hash" ]; then
            jq -n \
                --arg pin_hash "$hash" \
                --arg pin_salt "$salt" \
                '{pin_hash:$pin_hash, pin_salt:$pin_salt, pin_plain:"", hide_games:true, lock_ra_settings:true, allow_panic_file:true, allowed_apps:[]}' > "$tmp"
        else
            jq -n \
                '{pin_hash:"", pin_salt:"", pin_plain:"0000", hide_games:true, lock_ra_settings:true, allow_panic_file:true, allowed_apps:[]}' > "$tmp"
            log_msg "sha256sum unavailable; using plaintext PIN fallback."
        fi
        mv "$tmp" "$CONFIG_FILE"
        log_msg "Created default config with PIN 0000."
    fi

    tmp="/tmp/kidmode_config_normalized.$$"
    jq '
        def coerce_bool($value; $default):
            if $value == true or $value == "true" or $value == 1 or $value == "1" then true
            elif $value == false or $value == "false" or $value == 0 or $value == "0" then false
            else $default
            end;
        .hide_games = coerce_bool(.hide_games; true)
        | .lock_ra_settings = coerce_bool(.lock_ra_settings; true)
        | .allow_panic_file = coerce_bool(.allow_panic_file; true)
        | .pin_plain = ((.pin_plain // "") | tostring)
        | .manager_app_dir = ((.manager_app_dir // "") | tostring)
        | .allowed_apps = ((.allowed_apps // []) | map(tostring) | unique)
    ' "$CONFIG_FILE" > "$tmp"
    mv "$tmp" "$CONFIG_FILE"

    track_manager_app_name
}

config_get_bool() {
    key="$1"
    default="$2"
    value="$(jq -r --arg key "$key" '
        if has($key) and .[$key] != null then .[$key] else empty end
    ' "$CONFIG_FILE")"
    if [ -z "$value" ]; then
        printf '%s\n' "$default"
    else
        printf '%s\n' "$value"
    fi
}

config_set_bool() {
    key="$1"
    value="$2"
    tmp="/tmp/kidmode_config.$$"
    jq --arg key "$key" --argjson value "$value" '.[$key]=$value' "$CONFIG_FILE" > "$tmp"
    mv "$tmp" "$CONFIG_FILE"
}

set_manager_app_name() {
    manager_name="$1"
    [ -n "$manager_name" ] || return 1
    tmp="/tmp/kidmode_config.$$"
    jq --arg manager_app_dir "$manager_name" '.manager_app_dir=$manager_app_dir' "$CONFIG_FILE" > "$tmp"
    mv "$tmp" "$CONFIG_FILE"
}

resolve_manager_app_name() {
    configured="$(jq -r '.manager_app_dir // empty' "$CONFIG_FILE" 2>/dev/null || true)"
    if [ -n "$configured" ] &&
        [ "$configured" != "$UNLOCKER_APP_NAME" ] &&
        ( [ -d "$APP_ROOT/$configured" ] || [ -d "$KM_BACKUP_DIR/App/$configured" ] || [ -d "$MANAGER_TEMPLATE_DIR" ] ); then
        printf '%s\n' "$configured"
        return 0
    fi

    current_name="$(basename "$SCRIPT_DIR")"
    if [ "$current_name" != "$UNLOCKER_APP_NAME" ] && [ -f "$SCRIPT_DIR/config.json" ]; then
        printf '%s\n' "$current_name"
        return 0
    fi

    for candidate in "$KIDMODE_APP_NAME" "$KIDMODE_APP_ALT_NAME"; do
        if [ -d "$APP_ROOT/$candidate" ] || [ -d "$KM_BACKUP_DIR/App/$candidate" ]; then
            printf '%s\n' "$candidate"
            return 0
        fi
    done

    for root in "$APP_ROOT" "$KM_BACKUP_DIR/App"; do
        [ -d "$root" ] || continue
        for app_dir_path in "$root"/*; do
            [ -d "$app_dir_path" ] || continue
            [ -f "$app_dir_path/config.json" ] || continue
            [ -f "$app_dir_path/kidmodectl.sh" ] || continue
            if jq -e '.label // "" | test("^Kids Mode")' "$app_dir_path/config.json" >/dev/null 2>&1; then
                basename "$app_dir_path"
                return 0
            fi
        done
    done

    printf '%s\n' "$KIDMODE_APP_NAME"
}

track_manager_app_name() {
    current_name="$(basename "$SCRIPT_DIR")"
    if [ "$current_name" = "$UNLOCKER_APP_NAME" ]; then
        return 0
    fi

    if [ ! -f "$SCRIPT_DIR/config.json" ]; then
        return 0
    fi

    set_manager_app_name "$current_name" || true
}

config_get_allowed_apps_count() {
    jq -r '(.allowed_apps // []) | length' "$CONFIG_FILE"
}

config_is_app_allowed() {
    app_dir="$1"
    jq -e --arg app "$app_dir" '((.allowed_apps // []) | index($app)) != null' "$CONFIG_FILE" >/dev/null 2>&1
}

config_toggle_allowed_app() {
    app_dir="$1"
    tmp="/tmp/kidmode_config.$$"
    jq --arg app "$app_dir" \
        '.allowed_apps = (
            (.allowed_apps // [])
            | map(tostring)
            | if index($app) == null then . + [$app] else map(select(. != $app)) end
            | unique
        )' \
        "$CONFIG_FILE" > "$tmp"
    mv "$tmp" "$CONFIG_FILE"
}

shell_quote() {
    printf "'%s'" "$(printf '%s' "$1" | sed "s/'/'\\\\''/g")"
}

snapshot_manager_template() {
    manager_name="$(resolve_manager_app_name)"
    manager_src="$APP_ROOT/$manager_name"
    if [ -d "$manager_src" ] && [ -f "$manager_src/config.json" ]; then
        rm -rf "$MANAGER_TEMPLATE_DIR"
        cp -a "$manager_src" "$MANAGER_TEMPLATE_DIR"
    fi
}

ensure_manager_app_present() {
    manager_name="$(resolve_manager_app_name)"
    [ -n "$manager_name" ] || manager_name="$KIDMODE_APP_NAME"

    if [ -d "$APP_ROOT/$manager_name" ] && [ -f "$APP_ROOT/$manager_name/config.json" ]; then
        return 0
    fi

    if [ -d "$MANAGER_TEMPLATE_DIR" ] && [ -f "$MANAGER_TEMPLATE_DIR/config.json" ]; then
        cp -a "$MANAGER_TEMPLATE_DIR" "$APP_ROOT/$manager_name"
        chmod +x "$APP_ROOT/$manager_name/launch.sh" 2>/dev/null || true
        chmod +x "$APP_ROOT/$manager_name/kidmodectl.sh" 2>/dev/null || true
        set_manager_app_name "$manager_name" || true
        log_msg "Restored missing $manager_name app from manager template."
        return 0
    fi

    app_dir="$APP_ROOT/$manager_name"
    mkdir -p "$app_dir/data" "$app_dir/unlocker"
    cp "$SCRIPT_DIR/kidmodectl.sh" "$app_dir/kidmodectl.sh" 2>/dev/null || true

    cat > "$app_dir/launch.sh" <<'EOF'
#!/bin/sh
progdir="$(CDPATH= cd -- "$(dirname "$0")" >/dev/null 2>&1 && pwd -P)"
cd "$progdir" || exit 1
./kidmodectl.sh manager
EOF

    cat > "$app_dir/config.json" <<'EOF'
{
    "label": "Kids Mode [OFF]",
    "icon": "../../Icons/Default/app/guest_off.png",
    "iconsel": "",
    "launch": "launch.sh",
    "description": "Lock apps and games behind a PIN"
}
EOF

    cat > "$app_dir/data/configOFF.json" <<'EOF'
{
    "label": "Kids Mode [OFF]",
    "icon": "../../Icons/Default/app/guest_off.png",
    "iconsel": "",
    "launch": "launch.sh",
    "description": "Lock apps and games behind a PIN"
}
EOF

    cat > "$app_dir/data/configON.json" <<'EOF'
{
    "label": "Kids Mode [ON]",
    "icon": "../../Icons/Default/app/guest_on.png",
    "iconsel": "",
    "launch": "launch.sh",
    "description": "Unlock to restore normal mode"
}
EOF

    chmod +x "$app_dir/launch.sh" "$app_dir/kidmodectl.sh" 2>/dev/null || true
    set_manager_app_name "$manager_name" || true
    log_msg "Recreated missing $manager_name app from fallback template."
    return 0
}

ensure_canonical_manager_alias() {
    canonical_dir="$APP_ROOT/$KIDMODE_APP_NAME"

    normalize_manager_app_dir() {
        app_dir="$1"
        [ -d "$app_dir" ] || return 1

        if [ -f "$app_dir/data/configOFF.json" ]; then
            cp "$app_dir/data/configOFF.json" "$app_dir/config.json" 2>/dev/null || true
        elif [ ! -f "$app_dir/config.json" ]; then
            cat > "$app_dir/config.json" <<'EOF'
{
    "label": "Kids Mode [OFF]",
    "icon": "../../Icons/Default/app/guest_off.png",
    "iconsel": "",
    "launch": "launch.sh",
    "description": "Lock apps and games behind a PIN"
}
EOF
        fi

        if [ ! -f "$app_dir/launch.sh" ]; then
            cat > "$app_dir/launch.sh" <<'EOF'
#!/bin/sh
progdir="$(CDPATH= cd -- "$(dirname "$0")" >/dev/null 2>&1 && pwd -P)"
cd "$progdir" || exit 1
./kidmodectl.sh manager
EOF
        fi

        if [ ! -f "$app_dir/kidmodectl.sh" ] && [ -f "$SCRIPT_DIR/kidmodectl.sh" ]; then
            cp "$SCRIPT_DIR/kidmodectl.sh" "$app_dir/kidmodectl.sh" 2>/dev/null || true
        fi

        chmod +x "$app_dir/launch.sh" "$app_dir/kidmodectl.sh" 2>/dev/null || true
        return 0
    }

    if [ -d "$canonical_dir" ] && [ -f "$canonical_dir/config.json" ]; then
        normalize_manager_app_dir "$canonical_dir" || true
        set_manager_app_name "$KIDMODE_APP_NAME" || true
        return 0
    fi

    legacy_dir="$APP_ROOT/$KIDMODE_APP_ALT_NAME"
    if [ -d "$legacy_dir" ] && [ -f "$legacy_dir/config.json" ]; then
        rm -rf "$canonical_dir"
        cp -a "$legacy_dir" "$canonical_dir"
        normalize_manager_app_dir "$canonical_dir" || true
        chmod +x "$canonical_dir/launch.sh" 2>/dev/null || true
        chmod +x "$canonical_dir/kidmodectl.sh" 2>/dev/null || true
        set_manager_app_name "$KIDMODE_APP_NAME" || true
        log_msg "Created canonical $KIDMODE_APP_NAME app from legacy directory."
        return 0
    fi

    if [ -d "$MANAGER_TEMPLATE_DIR" ] && [ -f "$MANAGER_TEMPLATE_DIR/config.json" ]; then
        rm -rf "$canonical_dir"
        cp -a "$MANAGER_TEMPLATE_DIR" "$canonical_dir"
        normalize_manager_app_dir "$canonical_dir" || true
        chmod +x "$canonical_dir/launch.sh" 2>/dev/null || true
        chmod +x "$canonical_dir/kidmodectl.sh" 2>/dev/null || true
        set_manager_app_name "$KIDMODE_APP_NAME" || true
        log_msg "Restored canonical $KIDMODE_APP_NAME app from manager template."
        return 0
    fi

    return 1
}

restore_manager_from_package_manager_source() {
    target_dir="$APP_ROOT/$KIDMODE_APP_NAME"
    pm_src_active="$APP_ROOT/PackageManager/data/App/Kids Mode/App/Kids_Mode"
    pm_src_backup="$KM_BACKUP_DIR/App/PackageManager/data/App/Kids Mode/App/Kids_Mode"

    if [ -d "$target_dir" ] && [ -f "$target_dir/config.json" ]; then
        return 0
    fi

    if [ -d "$pm_src_active" ] && [ -f "$pm_src_active/config.json" ]; then
        rm -rf "$target_dir"
        cp -a "$pm_src_active" "$target_dir"
        chmod +x "$target_dir/launch.sh" "$target_dir/kidmodectl.sh" 2>/dev/null || true
        set_manager_app_name "$KIDMODE_APP_NAME" || true
        log_msg "Restored $KIDMODE_APP_NAME from active PackageManager source."
        return 0
    fi

    if [ -d "$pm_src_backup" ] && [ -f "$pm_src_backup/config.json" ]; then
        rm -rf "$target_dir"
        cp -a "$pm_src_backup" "$target_dir"
        chmod +x "$target_dir/launch.sh" "$target_dir/kidmodectl.sh" 2>/dev/null || true
        set_manager_app_name "$KIDMODE_APP_NAME" || true
        log_msg "Restored $KIDMODE_APP_NAME from backup PackageManager source."
        return 0
    fi

    return 1
}

set_pin_value() {
    pin="$1"
    if ! is_valid_pin "$pin"; then
        echo "kidmodectl: PIN must be 4 digits" >&2
        return 1
    fi

    salt="$(generate_salt)"
    tmp="/tmp/kidmode_config.$$"

    hash="$(hash_pin "$pin" "$salt" 2>/dev/null || true)"
    if [ -n "$hash" ]; then
        jq --arg pin_hash "$hash" --arg pin_salt "$salt" \
            '.pin_hash=$pin_hash | .pin_salt=$pin_salt | .pin_plain=""' \
            "$CONFIG_FILE" > "$tmp"
    else
        jq --arg pin_plain "$pin" \
            '.pin_hash="" | .pin_salt="" | .pin_plain=$pin_plain' \
            "$CONFIG_FILE" > "$tmp"
        log_msg "sha256sum unavailable; stored plaintext PIN fallback."
    fi

    mv "$tmp" "$CONFIG_FILE"
    log_msg "PIN updated."
    return 0
}

verify_pin() {
    pin="$1"
    salt="$(jq -r '.pin_salt // empty' "$CONFIG_FILE")"
    stored_hash="$(jq -r '.pin_hash // empty' "$CONFIG_FILE")"
    stored_plain="$(jq -r '.pin_plain // empty' "$CONFIG_FILE")"

    if [ -n "$salt" ] && [ -n "$stored_hash" ]; then
        entered_hash="$(hash_pin "$pin" "$salt" 2>/dev/null || true)"
        if [ -n "$entered_hash" ] && [ "$entered_hash" = "$stored_hash" ]; then
            return 0
        fi
    fi

    if is_valid_pin "$stored_plain" && [ "$pin" = "$stored_plain" ]; then
        return 0
    fi

    return 1
}

has_valid_pin_config() {
    salt="$(jq -r '.pin_salt // empty' "$CONFIG_FILE")"
    stored_hash="$(jq -r '.pin_hash // empty' "$CONFIG_FILE")"
    stored_plain="$(jq -r '.pin_plain // empty' "$CONFIG_FILE")"
    if [ -n "$salt" ] && [ -n "$stored_hash" ]; then
        return 0
    fi
    is_valid_pin "$stored_plain"
}

manager_config_path() {
    manager_name="$(resolve_manager_app_name)"
    manager_path="$APP_ROOT/$manager_name/config.json"
    backup_path="$KM_BACKUP_DIR/App/$manager_name/config.json"

    if [ -f "$manager_path" ]; then
        printf '%s\n' "$manager_path"
        return 0
    fi

    if [ -f "$backup_path" ]; then
        printf '%s\n' "$backup_path"
        return 0
    fi

    printf '%s\n' "$manager_path"
}

set_manager_state() {
    state="$1"
    config_path="$(manager_config_path)"

    if [ "$state" = "on" ]; then
        label="Kids Mode [ON]"
        icon="$DEFAULT_ICON_ON"
        description="Unlock to restore normal mode"
    else
        label="Kids Mode [OFF]"
        icon="$DEFAULT_ICON_OFF"
        description="Lock apps and games behind a PIN"
    fi

    mkdir -p "$(dirname "$config_path")"
    cat > "$config_path" <<EOF
{
    "label": "$label",
    "icon": "$icon",
    "iconsel": "",
    "launch": "launch.sh",
    "description": "$description"
}
EOF
}

acquire_operation_lock() {
    mkdir -p "$KM_ROOT"
    if mkdir "$KM_LOCKDIR" 2>/dev/null; then
        printf '%s\n' "$$" > "$KM_LOCKDIR/pid"
        return 0
    fi

    if [ -f "$KM_LOCKDIR/pid" ]; then
        stale_pid="$(cat "$KM_LOCKDIR/pid" 2>/dev/null || true)"
        if [ -n "$stale_pid" ] && ! kill -0 "$stale_pid" 2>/dev/null; then
            rm -rf "$KM_LOCKDIR"
            if mkdir "$KM_LOCKDIR" 2>/dev/null; then
                printf '%s\n' "$$" > "$KM_LOCKDIR/pid"
                log_msg "Recovered stale operation lock from pid=$stale_pid."
                return 0
            fi
        fi
    fi

    log_msg "Unable to acquire operation lock."
    return 1
}

release_operation_lock() {
    rm -f "$KM_LOCKDIR/pid" 2>/dev/null || true
    rmdir "$KM_LOCKDIR" 2>/dev/null || true
}

with_lock() {
    if ! acquire_operation_lock; then
        echo "Kids Mode operation already in progress. Try again." >&2
        return 1
    fi

    trap 'release_operation_lock' EXIT INT TERM
    "$@"
    rc=$?
    trap - EXIT INT TERM
    release_operation_lock

    return "$rc"
}

is_locked() {
    [ -d "$KM_BACKUP_DIR/App" ]
}

refresh_ui() {
    if [ -x "$RESET_LIST_SH" ]; then
        "$RESET_LIST_SH" "$APP_ROOT" >/dev/null 2>&1 || true
        "$RESET_LIST_SH" "$ROMS_ROOT" >/dev/null 2>&1 || true
        "$RESET_LIST_SH" "$EMU_ROOT" >/dev/null 2>&1 || true
        "$RESET_LIST_SH" "$APP_ROOT/$KIDMODE_APP_NAME" >/dev/null 2>&1 || true
        "$RESET_LIST_SH" "$APP_ROOT/$UNLOCKER_APP_NAME" >/dev/null 2>&1 || true
        "$RESET_LIST_SH" "$KM_BACKUP_DIR/App" >/dev/null 2>&1 || true
        "$RESET_LIST_SH" "$KM_BACKUP_DIR/Roms" >/dev/null 2>&1 || true
        "$RESET_LIST_SH" "$KM_BACKUP_DIR/Emu" >/dev/null 2>&1 || true
    fi

    rm -f "$APP_ROOT/App_cache2.db" "$APP_ROOT/App_cache6.db" 2>/dev/null || true
    rm -f "$ROMS_ROOT/Roms_cache2.db" "$ROMS_ROOT/Roms_cache6.db" 2>/dev/null || true
    rm -f "$EMU_ROOT/Emu_cache2.db" "$EMU_ROOT/Emu_cache6.db" 2>/dev/null || true
    rm -f "$KM_BACKUP_DIR/App/App_cache2.db" "$KM_BACKUP_DIR/App/App_cache6.db" 2>/dev/null || true
    rm -f "$KM_BACKUP_DIR/Roms/Roms_cache2.db" "$KM_BACKUP_DIR/Roms/Roms_cache6.db" 2>/dev/null || true
    rm -f "$KM_BACKUP_DIR/Emu/Emu_cache2.db" "$KM_BACKUP_DIR/Emu/Emu_cache6.db" 2>/dev/null || true
    rm -f /tmp/state.json /appconfigs/romwinidx.json 2>/dev/null || true

    if command -v killall >/dev/null 2>&1; then
        killall MainUI >/dev/null 2>&1 || true
    elif pgrep "MainUI" >/dev/null 2>&1; then
        pkill "MainUI" >/dev/null 2>&1 || true
    fi

    sleep 1
    if pgrep "MainUI" >/dev/null 2>&1; then
        pkill -9 "MainUI" >/dev/null 2>&1 || true
    fi
}

ra_set_option() {
    key="$1"
    value="$2"
    config_path="$3"

    escaped_key="$(printf '%s' "$key" | sed 's/[][\\/.*^$]/\\&/g')"
    if grep -q "^[[:space:]]*$key[[:space:]]*=" "$config_path" 2>/dev/null; then
        sed -i "s|^[[:space:]]*${escaped_key}[[:space:]]*=.*|${key} = \"${value}\"|" "$config_path"
    else
        printf '%s = "%s"\n' "$key" "$value" >> "$config_path"
    fi
}

apply_retroarch_kids_lock() {
    if [ ! -f "$RA_CONFIG_FILE" ]; then
        return 0
    fi

    lock_ra_settings="$(config_get_bool "lock_ra_settings" "true")"
    if [ "$lock_ra_settings" != "true" ]; then
        return 0
    fi

    if [ ! -f "$RA_BACKUP_FILE" ]; then
        cp "$RA_CONFIG_FILE" "$RA_BACKUP_FILE"
    fi

    ra_set_option "kiosk_mode_enable" "true" "$RA_CONFIG_FILE"
    ra_set_option "quick_menu_show_options" "false" "$RA_CONFIG_FILE"
    ra_set_option "quick_menu_show_cheats" "false" "$RA_CONFIG_FILE"
    ra_set_option "quick_menu_show_shaders" "false" "$RA_CONFIG_FILE"
    ra_set_option "quick_menu_show_start_recording" "false" "$RA_CONFIG_FILE"
    ra_set_option "quick_menu_show_start_streaming" "false" "$RA_CONFIG_FILE"
    ra_set_option "settings_show_configuration" "false" "$RA_CONFIG_FILE"
    ra_set_option "settings_show_core" "false" "$RA_CONFIG_FILE"
    ra_set_option "settings_show_directory" "false" "$RA_CONFIG_FILE"
    ra_set_option "settings_show_drivers" "false" "$RA_CONFIG_FILE"
    ra_set_option "settings_show_file_browser" "false" "$RA_CONFIG_FILE"
    ra_set_option "settings_show_input" "false" "$RA_CONFIG_FILE"
    ra_set_option "settings_show_latency" "false" "$RA_CONFIG_FILE"
    ra_set_option "settings_show_network" "false" "$RA_CONFIG_FILE"
    ra_set_option "settings_show_recording" "false" "$RA_CONFIG_FILE"
    ra_set_option "settings_show_user" "false" "$RA_CONFIG_FILE"
    ra_set_option "settings_show_user_interface" "false" "$RA_CONFIG_FILE"
    ra_set_option "settings_show_video" "false" "$RA_CONFIG_FILE"
    ra_set_option "settings_show_audio" "false" "$RA_CONFIG_FILE"
}

restore_retroarch_from_backup() {
    if [ -f "$RA_BACKUP_FILE" ]; then
        cp "$RA_BACKUP_FILE" "$RA_CONFIG_FILE"
        rm -f "$RA_BACKUP_FILE"
    fi
}

safe_remove_kiosk_app() {
    if [ ! -d "$APP_ROOT" ]; then
        return 0
    fi

    if [ -f "$APP_ROOT/$KM_SENTINEL" ]; then
        rm -rf "$APP_ROOT"
        return 0
    fi

    # If backup exists but current App is not kiosk, archive it and continue restore.
    if [ -d "$KM_BACKUP_DIR/App" ]; then
        conflict_dir="$KM_ROOT/conflicts"
        mkdir -p "$conflict_dir"
        conflict_path="$conflict_dir/App.$(date +%s)"
        if mv "$APP_ROOT" "$conflict_path"; then
            log_msg "Archived non-kiosk App directory to $conflict_path during unlock."
            return 0
        fi
    fi

    echo "Kid Mode: refusing to remove $APP_ROOT without sentinel" >&2
    return 1
}

prepare_unlocker_stage() {
    rm -rf "$KM_STAGE_DIR"
    mkdir -p "$KM_STAGE_DIR/App"

    manager_name="$(resolve_manager_app_name)"
    [ -n "$manager_name" ] || manager_name="$KIDMODE_APP_NAME"
    manager_src="$APP_ROOT/$manager_name"
    staged_manager_dir="$KM_STAGE_DIR/App/$KIDMODE_APP_NAME"

    if [ -d "$manager_src" ] && [ -f "$manager_src/config.json" ]; then
        cp -a "$manager_src" "$staged_manager_dir"
    elif [ -d "$MANAGER_TEMPLATE_DIR" ] && [ -f "$MANAGER_TEMPLATE_DIR/config.json" ]; then
        cp -a "$MANAGER_TEMPLATE_DIR" "$staged_manager_dir"
    elif [ -d "$UNLOCKER_TEMPLATE_DIR" ]; then
        mkdir -p "$staged_manager_dir"
        cp -a "$UNLOCKER_TEMPLATE_DIR/." "$staged_manager_dir/"
    else
        echo "Kid Mode unlocker template is missing: $UNLOCKER_TEMPLATE_DIR" >&2
        return 1
    fi

    if [ ! -d "$staged_manager_dir" ]; then
        echo "Kid Mode: failed to stage manager app for lock." >&2
        return 1
    fi

    cat > "$staged_manager_dir/launch.sh" <<'EOF'
#!/bin/sh
progdir="$(CDPATH= cd -- "$(dirname "$0")" >/dev/null 2>&1 && pwd -P)"
cd "$progdir" || exit 1
./kidmodectl.sh unlock-interactive
EOF

    cp "$SCRIPT_DIR/kidmodectl.sh" "$staged_manager_dir/kidmodectl.sh"
    chmod +x "$staged_manager_dir/launch.sh" "$staged_manager_dir/kidmodectl.sh"

    if [ -f "$staged_manager_dir/data/configON.json" ]; then
        cp "$staged_manager_dir/data/configON.json" "$staged_manager_dir/config.json" 2>/dev/null || true
    else
        cat > "$staged_manager_dir/config.json" <<'EOF'
{
    "label": "Kids Mode [ON]",
    "icon": "../../Icons/Default/app/guest_on.png",
    "iconsel": "",
    "launch": "launch.sh",
    "description": "Unlock to restore normal mode"
}
EOF
    fi

    jq -r '.allowed_apps[]? // empty' "$CONFIG_FILE" | while IFS= read -r app_dir; do
        if [ -z "$app_dir" ] ||
            [ "$app_dir" = "$UNLOCKER_APP_NAME" ] ||
            [ "$app_dir" = "$manager_name" ] ||
            [ "$app_dir" = "$KIDMODE_APP_NAME" ] ||
            [ "$app_dir" = "$KIDMODE_APP_ALT_NAME" ]; then
            continue
        fi
        app_src="$APP_ROOT/$app_dir"
        if [ -d "$app_src" ] && [ -f "$app_src/config.json" ]; then
            cp -a "$app_src" "$KM_STAGE_DIR/App/"
        fi
    done

    touch "$KM_STAGE_DIR/App/$KM_SENTINEL"
}

rollback_lock() {
    log_msg "Running lock rollback."

    if [ -d "$APP_ROOT" ] && [ -f "$APP_ROOT/$KM_SENTINEL" ]; then
        rm -rf "$APP_ROOT"
    fi

    if [ -d "$KM_BACKUP_DIR/App" ] && [ ! -d "$APP_ROOT" ]; then
        mv "$KM_BACKUP_DIR/App" "$APP_ROOT" || true
    fi

    if [ -d "$KM_BACKUP_DIR/Roms" ] && [ ! -d "$ROMS_ROOT" ]; then
        mv "$KM_BACKUP_DIR/Roms" "$ROMS_ROOT" || true
    fi

    if [ -d "$KM_BACKUP_DIR/Emu" ] && [ ! -d "$EMU_ROOT" ]; then
        mv "$KM_BACKUP_DIR/Emu" "$EMU_ROOT" || true
    fi

    restore_retroarch_from_backup

    rm -rf "$KM_STAGE_DIR"
    set_manager_state "off"
    state_write "idle" "Lock rollback completed."
}

command_lock() {
    ensure_layout
    track_manager_app_name
    state_write "locking" "Starting lock operation."

    # Ensure manager app is present before creating a new locked snapshot.
    ensure_manager_app_present || true
    ensure_canonical_manager_alias || true

    if [ ! -d "$APP_ROOT/$KIDMODE_APP_NAME" ] || [ ! -f "$APP_ROOT/$KIDMODE_APP_NAME/config.json" ]; then
        state_write "error" "Manager app missing; cannot lock safely."
        echo "Kid Mode: missing manager app at $APP_ROOT/$KIDMODE_APP_NAME" >&2
        return 1
    fi

    if is_locked; then
        state_write "locked" "Already locked."
        return 0
    fi

    if [ ! -d "$APP_ROOT" ]; then
        state_write "error" "App directory missing; cannot lock."
        echo "Kid Mode: missing $APP_ROOT" >&2
        return 1
    fi

    if [ -d "$KM_BACKUP_DIR/App" ]; then
        state_write "error" "Backup already exists; recover required before lock."
        echo "Kid Mode: stale backup exists in $KM_BACKUP_DIR/App" >&2
        return 1
    fi

    if [ -d "$KM_BACKUP_DIR/Roms" ]; then
        state_write "error" "Roms backup already exists; recover required before lock."
        echo "Kid Mode: stale backup exists in $KM_BACKUP_DIR/Roms" >&2
        return 1
    fi

    if [ -d "$KM_BACKUP_DIR/Emu" ]; then
        state_write "error" "Emu backup already exists; recover required before lock."
        echo "Kid Mode: stale backup exists in $KM_BACKUP_DIR/Emu" >&2
        return 1
    fi

    hide_games="$(config_get_bool "hide_games" "true")"
    snapshot_manager_template

    if ! prepare_unlocker_stage; then
        state_write "error" "Unable to prepare unlocker stage."
        return 1
    fi

    set_manager_state "on"

    if ! mv "$APP_ROOT" "$KM_BACKUP_DIR/App"; then
        rollback_lock
        state_write "error" "Failed moving App to backup."
        return 1
    fi

    if [ "$hide_games" = "true" ] && [ -d "$ROMS_ROOT" ]; then
        if ! mv "$ROMS_ROOT" "$KM_BACKUP_DIR/Roms"; then
            rollback_lock
            state_write "error" "Failed moving Roms to backup."
            return 1
        fi
    fi

    if [ "$hide_games" = "true" ] && [ -d "$EMU_ROOT" ]; then
        if ! mv "$EMU_ROOT" "$KM_BACKUP_DIR/Emu"; then
            rollback_lock
            state_write "error" "Failed moving Emu to backup."
            return 1
        fi
    fi

    if ! mv "$KM_STAGE_DIR/App" "$APP_ROOT"; then
        rollback_lock
        state_write "error" "Failed moving kiosk App into place."
        return 1
    fi

    apply_retroarch_kids_lock || true

    rm -rf "$KM_STAGE_DIR"
    sync
    refresh_ui

    state_write "locked" "Kids Mode is active."
    log_msg "Kids Mode locked successfully."
    return 0
}

command_unlock() {
    ensure_layout
    state_write "unlocking" "Starting unlock operation."

    if ! is_locked; then
        state_write "idle" "Already unlocked."
        return 0
    fi

    set_manager_state "off"

    if ! safe_remove_kiosk_app; then
        state_write "error" "Refusing to remove non-kiosk App directory."
        return 1
    fi

    if ! mv "$KM_BACKUP_DIR/App" "$APP_ROOT"; then
        state_write "error" "Failed restoring App directory."
        return 1
    fi

    restore_manager_from_package_manager_source || true
    ensure_manager_app_present || true
    ensure_canonical_manager_alias || true

    manager_dir="$APP_ROOT/$KIDMODE_APP_NAME"
    if [ -d "$manager_dir" ]; then
        cp "$SCRIPT_DIR/kidmodectl.sh" "$manager_dir/kidmodectl.sh" 2>/dev/null || true
        chmod +x "$manager_dir/launch.sh" "$manager_dir/kidmodectl.sh" 2>/dev/null || true
    fi
    set_manager_state "off"
    snapshot_manager_template

    if [ -d "$KM_BACKUP_DIR/Roms" ]; then
        if [ -e "$ROMS_ROOT" ]; then
            conflict_dir="$KM_ROOT/conflicts"
            mkdir -p "$conflict_dir"
            conflict_path="$conflict_dir/Roms.$(date +%s)"
            mv "$ROMS_ROOT" "$conflict_path" || true
            log_msg "Moved conflicting $ROMS_ROOT to $conflict_path"
        fi

        if ! mv "$KM_BACKUP_DIR/Roms" "$ROMS_ROOT"; then
            state_write "error" "Failed restoring Roms directory."
            return 1
        fi
    fi

    if [ -d "$KM_BACKUP_DIR/Emu" ]; then
        if [ -e "$EMU_ROOT" ]; then
            conflict_dir="$KM_ROOT/conflicts"
            mkdir -p "$conflict_dir"
            conflict_path="$conflict_dir/Emu.$(date +%s)"
            mv "$EMU_ROOT" "$conflict_path" || true
            log_msg "Moved conflicting $EMU_ROOT to $conflict_path"
        fi

        if ! mv "$KM_BACKUP_DIR/Emu" "$EMU_ROOT"; then
            state_write "error" "Failed restoring Emu directory."
            return 1
        fi
    fi

    restore_retroarch_from_backup

    rm -rf "$KM_STAGE_DIR"
    sync
    refresh_ui

    state_write "idle" "Kids Mode is inactive."
    log_msg "Kids Mode unlocked successfully."
    return 0
}

command_recover() {
    ensure_layout
    state_write "recovering" "Checking for incomplete operations."

    if [ -d "$KM_BACKUP_DIR/App" ] && [ ! -d "$APP_ROOT" ]; then
        log_msg "Recovering by restoring App from backup."
        mv "$KM_BACKUP_DIR/App" "$APP_ROOT"
        restore_manager_from_package_manager_source || true
        ensure_manager_app_present || true
        ensure_canonical_manager_alias || true

        manager_dir="$APP_ROOT/$KIDMODE_APP_NAME"
        if [ -d "$manager_dir" ]; then
            cp "$SCRIPT_DIR/kidmodectl.sh" "$manager_dir/kidmodectl.sh" 2>/dev/null || true
            chmod +x "$manager_dir/launch.sh" "$manager_dir/kidmodectl.sh" 2>/dev/null || true
        fi

        if [ -d "$KM_BACKUP_DIR/Roms" ] && [ ! -d "$ROMS_ROOT" ]; then
            mv "$KM_BACKUP_DIR/Roms" "$ROMS_ROOT"
        fi

        if [ -d "$KM_BACKUP_DIR/Emu" ] && [ ! -d "$EMU_ROOT" ]; then
            mv "$KM_BACKUP_DIR/Emu" "$EMU_ROOT"
        fi

        restore_retroarch_from_backup

        rm -rf "$KM_STAGE_DIR"
        set_manager_state "off"
        sync
        refresh_ui
        state_write "idle" "Recovered from incomplete lock."
        return 0
    fi

    if [ -d "$APP_ROOT" ] && [ -f "$APP_ROOT/$KM_SENTINEL" ] && [ -d "$KM_BACKUP_DIR/App" ]; then
        rm -rf "$KM_STAGE_DIR"
        state_write "locked" "Locked state is consistent."
        return 0
    fi

    if [ -d "$APP_ROOT" ] && [ ! -f "$APP_ROOT/$KM_SENTINEL" ] && [ -d "$KM_BACKUP_DIR/App" ]; then
        state_write "warning" "Backup exists while App is not kiosk; manual review required."
        echo "Kid Mode: warning - backup exists but App is not kiosk." >&2
        return 2
    fi

    rm -rf "$KM_STAGE_DIR"
    state_write "idle" "No recovery action required."
    return 0
}

run_prompt() {
    if [ ! -x "$PROMPT_BIN" ]; then
        return 255
    fi

    LD_PRELOAD="$LD_PRELOAD_LIB" "$PROMPT_BIN" "$@"
}

run_kbinput() {
    if [ ! -x "$KBINPUT_BIN" ]; then
        return 255
    fi

    (
        cd "$SDCARD/.tmp_update" || exit 255
        LD_PRELOAD="$LD_PRELOAD_LIB" ./bin/kbinput "$@"
    )
}

run_pinpad() {
    if [ ! -x "$PINPAD_BIN" ]; then
        return 255
    fi

    LD_PRELOAD="$LD_PRELOAD_LIB" "$PINPAD_BIN" "$@"
}

show_info() {
    title="$1"
    message="$2"
    run_prompt -t "$title" -m "$message" "OK" >/dev/null 2>&1 || true
}

prompt_text_kb() {
    title="$1"
    initial="${2:-}"

    tmp="/tmp/kidmode_kbinput.$$"
    run_kbinput -i "$initial" -t "$title" > "$tmp"
    ret=$?

    value="$(tail -n 1 "$tmp" 2>/dev/null | tr -d '\r')"
    rm -f "$tmp"

    if [ "$ret" -ne 0 ]; then
        return 1
    fi

    printf '%s\n' "$value"
}

prompt_text_pinpad() {
    title="$1"
    message="$2"
    initial="${3:-}"

    tmp="/tmp/kidmode_pinpad.$$"
    run_pinpad -t "$title" -m "$message" -i "$initial" > "$tmp"
    ret=$?

    # Prefer explicit pinpad output format, then strict 4-digit line.
    value="$(grep -E '^PIN:[0-9]{4}$' "$tmp" 2>/dev/null | tail -n 1 | cut -d: -f2 | tr -d '\r' || true)"
    if [ -z "$value" ]; then
        value="$(grep -E '^[0-9]{4}$' "$tmp" 2>/dev/null | tail -n 1 | tr -d '\r' || true)"
    fi
    if [ -z "$value" ]; then
        value="$(tail -n 1 "$tmp" 2>/dev/null | tr -d '\r')"
    fi
    rm -f "$tmp"

    if [ "$ret" -ne 0 ]; then
        return 1
    fi

    printf '%s\n' "$value"
}

prompt_digit() {
    title="$1"
    position="$2"
    masked="$3"
    prompt_message="Select digit ${position}/4"

    if [ -n "$masked" ]; then
        prompt_message="${prompt_message}\\nEntered: ${masked}"
    fi

    run_prompt -t "$title" -m "$prompt_message" \
        "0" "1" "2" "3" "4" "5" "6" "7" "8" "9" "Cancel"
    ret=$?

    case "$ret" in
        0|1|2|3|4|5|6|7|8|9)
            printf '%s\n' "$ret"
            return 0
            ;;
        *)
            return 1
            ;;
    esac
}

prompt_pin_code_legacy() {
    title="$1"
    pin=""
    idx=1

    while [ "$idx" -le 4 ]; do
        masked=""
        i=1
        while [ "$i" -le "${#pin}" ]; do
            masked="${masked}*"
            i=$((i + 1))
        done

        digit="$(prompt_digit "$title" "$idx" "$masked")" || return 1
        pin="${pin}${digit}"
        idx=$((idx + 1))
    done

    printf '%s\n' "$pin"
}

prompt_pin_code_kb() {
    title="$1 (4 digits, START=OK)"

    while true; do
        raw_value="$(prompt_text_kb "$title" "")" || return 1
        pin="$(printf '%s' "$raw_value" | tr -cd '0-9')"

        if [ "${#pin}" -eq 4 ]; then
            printf '%s\n' "$pin"
            return 0
        fi

        show_info "Kids Mode" "PIN must be exactly 4 digits."
    done
}

prompt_pin_code_spinner() {
    title="$1"
    message=""

    raw_value="$(prompt_text_pinpad "$title" "$message" "0000")" || return 1
    pin="$(printf '%s' "$raw_value" | tr -d '\r\n\t ')"
    case "$pin" in
        PIN:[0-9][0-9][0-9][0-9])
            pin="${pin#PIN:}"
            ;;
    esac
    if [ -z "$pin" ]; then
        pin="$(printf '%s' "$raw_value" | tr -cd '0-9')"
    fi

    if [ "${#pin}" -eq 4 ]; then
        printf '%s\n' "$pin"
        return 0
    fi

    return 1
}

prompt_pin_code() {
    title="$1"
    if [ -x "$PINPAD_BIN" ]; then
        spinner_pin="$(prompt_pin_code_spinner "$title" || true)"
        if is_valid_pin "$spinner_pin"; then
            printf '%s\n' "$spinner_pin"
            return 0
        fi
    fi

    if [ -x "$KBINPUT_BIN" ]; then
        prompt_pin_code_kb "$title"
    else
        prompt_pin_code_legacy "$title"
    fi
}

command_set_pin_interactive() {
    ensure_layout

    pin1="$(prompt_pin_code "Kids Mode: New PIN")" || {
        show_info "Kids Mode" "PIN setup canceled."
        return 1
    }
    pin2="$(prompt_pin_code "Kids Mode: Confirm PIN")" || {
        show_info "Kids Mode" "PIN setup canceled."
        return 1
    }

    if [ "$pin1" != "$pin2" ]; then
        show_info "Kids Mode" "PIN entries did not match."
        return 1
    fi

    if set_pin_value "$pin1"; then
        show_info "Kids Mode" "PIN updated successfully."
        return 0
    fi

    show_info "Kids Mode" "Failed to update PIN."
    return 1
}

command_unlock_interactive() {
    ensure_layout

    if ! is_locked; then
        show_info "Kids Mode" "Kids Mode is already unlocked."
        return 0
    fi

    allow_panic="$(config_get_bool "allow_panic_file" "true")"
    panic_token=""
    if [ -f "$PANIC_FILE" ]; then
        panic_token="$(tr -d '\r\n\t ' < "$PANIC_FILE" 2>/dev/null || true)"
    fi
    if [ "$allow_panic" = "true" ] && [ -f "$PANIC_FILE" ] && [ "$panic_token" = "RESET_KIDMODE" ]; then
        rm -f "$PANIC_FILE"
        log_msg "Panic file detected; bypassing PIN check."
        with_lock command_unlock
        return $?
    fi

    attempt=1
    while [ "$attempt" -le 3 ]; do
        entered_pin="$(prompt_pin_code "Kids Unlock (Attempt ${attempt}/3)")" || {
            show_info "Kids Mode" "Unlock canceled."
            return 1
        }

        if verify_pin "$entered_pin"; then
            with_lock command_unlock
            rc=$?
            if [ "$rc" -eq 0 ]; then
                show_info "Kids Mode" "Unlocked successfully."
            fi
            return "$rc"
        fi

        show_info "Kids Mode" "Incorrect PIN."
        attempt=$((attempt + 1))
    done

    show_info "Kids Mode" "Too many failed attempts."
    return 1
}

command_toggle_hide_games() {
    ensure_layout

    current="$(config_get_bool "hide_games" "true")"
    if [ "$current" = "true" ]; then
        config_set_bool "hide_games" "false"
        show_info "Kids Mode" "Hide Games set to OFF\\n(applies on next lock)."
    else
        config_set_bool "hide_games" "true"
        show_info "Kids Mode" "Hide Games set to ON\\n(applies on next lock)."
    fi
}

command_toggle_lock_ra_settings() {
    ensure_layout

    current="$(config_get_bool "lock_ra_settings" "true")"
    if [ "$current" = "true" ]; then
        config_set_bool "lock_ra_settings" "false"
        show_info "Kids Mode" "Lock RetroArch Settings set to OFF\\n(applies on next lock)."
    else
        config_set_bool "lock_ra_settings" "true"
        show_info "Kids Mode" "Lock RetroArch Settings set to ON\\n(applies on next lock)."
    fi
}

command_manage_allowed_apps() {
    ensure_layout

    while true; do
        manager_name="$(resolve_manager_app_name)"
        tmp="/tmp/kidmode_allowed_apps.$$"
        allowed_cache="/tmp/kidmode_allowed_cache.$$"
        : > "$tmp"
        jq -r '.allowed_apps[]? // empty' "$CONFIG_FILE" > "$allowed_cache"

        for app_dir_path in "$APP_ROOT"/*; do
            [ -d "$app_dir_path" ] || continue
            app_dir="$(basename "$app_dir_path")"
            [ "$app_dir" = "$UNLOCKER_APP_NAME" ] && continue
            [ "$app_dir" = "$manager_name" ] && continue
            [ -f "$app_dir_path/config.json" ] || continue

            label="$(jq -r '.label // empty' "$app_dir_path/config.json" 2>/dev/null)"
            [ -n "$label" ] || label="$app_dir"

            if grep -Fxq "$app_dir" "$allowed_cache"; then
                state="[ON]"
            else
                state="[OFF]"
            fi

            printf '%s|%s %s\n' "$app_dir" "$state" "$label" >> "$tmp"
        done

        if [ ! -s "$tmp" ]; then
            rm -f "$tmp"
            rm -f "$allowed_cache"
            show_info "Allowed Apps" "No apps available to allow."
            return 0
        fi

        cmd="run_prompt -t $(shell_quote "Allowed Apps") -m $(shell_quote "Select an app to toggle allowed state.")"
        while IFS='|' read -r _app_dir app_label; do
            cmd="$cmd $(shell_quote "$app_label")"
        done < "$tmp"
        cmd="$cmd $(shell_quote "Done")"
        eval "$cmd"
        action=$?

        app_count="$(wc -l < "$tmp" | tr -d '[:space:]')"
        if [ "$action" -lt 0 ] || [ "$action" -ge "$app_count" ]; then
            rm -f "$tmp"
            rm -f "$allowed_cache"
            return 0
        fi

        selected_app="$(sed -n "$((action + 1))p" "$tmp" | cut -d'|' -f1)"
        rm -f "$tmp"
        rm -f "$allowed_cache"

        if [ -n "$selected_app" ]; then
            config_toggle_allowed_app "$selected_app"
        fi
    done
}

command_status() {
    ensure_layout

    phase="$(jq -r '.phase // "unknown"' "$KM_STATE_FILE")"
    hide_games="$(config_get_bool "hide_games" "true")"
    lock_ra_settings="$(config_get_bool "lock_ra_settings" "true")"
    allow_panic="$(config_get_bool "allow_panic_file" "true")"
    allowed_apps_count="$(config_get_allowed_apps_count)"

    if is_locked; then
        locked="true"
    else
        locked="false"
    fi

    if [ -f "$APP_ROOT/$KM_SENTINEL" ]; then
        sentinel_present="true"
    else
        sentinel_present="false"
    fi

    if [ -d "$KM_BACKUP_DIR/App" ]; then
        backup_app_present="true"
    else
        backup_app_present="false"
    fi

    if [ -d "$ROMS_ROOT" ]; then
        roms_present="true"
    else
        roms_present="false"
    fi

    if [ -d "$EMU_ROOT" ]; then
        emu_present="true"
    else
        emu_present="false"
    fi

    if [ -d "$KM_BACKUP_DIR/Roms" ]; then
        roms_hidden="true"
    else
        roms_hidden="false"
    fi

    if [ -d "$KM_BACKUP_DIR/Emu" ]; then
        emu_hidden="true"
    else
        emu_hidden="false"
    fi

    manager_name="$(resolve_manager_app_name)"
    if [ ! -d "$APP_ROOT/$manager_name" ] && [ -d "$APP_ROOT/$KIDMODE_APP_NAME" ]; then
        manager_name="$KIDMODE_APP_NAME"
    fi
    if [ -d "$APP_ROOT/$manager_name" ]; then
        manager_app_present="true"
    else
        manager_app_present="false"
    fi

    if [ -f "$PANIC_FILE" ]; then
        panic_present="true"
    else
        panic_present="false"
    fi

    if [ -f "$RA_BACKUP_FILE" ]; then
        ra_locked="true"
    else
        ra_locked="false"
    fi

    echo "locked=$locked"
    echo "phase=$phase"
    echo "hide_games=$hide_games"
    echo "lock_ra_settings=$lock_ra_settings"
    echo "allowed_apps_count=$allowed_apps_count"
    echo "allow_panic_file=$allow_panic"
    echo "manager_app_dir=$manager_name"
    echo "manager_app_present=$manager_app_present"
    echo "backup_app_present=$backup_app_present"
    echo "kiosk_sentinel_present=$sentinel_present"
    echo "roms_present=$roms_present"
    echo "emu_present=$emu_present"
    echo "roms_hidden=$roms_hidden"
    echo "emu_hidden=$emu_hidden"
    echo "ra_locked=$ra_locked"
    echo "panic_file_present=$panic_present"
}

command_manager_menu() {
    ensure_layout
    track_manager_app_name

    while true; do
        if is_locked; then
            run_prompt -t "Kids Mode [ON]" -m "Kids Mode is active." \
                "Unlock (enter PIN)" \
                "Recover" \
                "Status" \
                "Cancel"
            action=$?

            case "$action" in
                0)
                    command_unlock_interactive
                    ;;
                1)
                    with_lock command_recover
                    ;;
                2)
                    status_lines="$(command_status | sed ':a;N;$!ba;s/\n/\\n/g')"
                    show_info "Kids Mode Status" "$status_lines"
                    ;;
                *)
                    return 0
                    ;;
            esac
            continue
        fi

        hide_games="$(config_get_bool "hide_games" "true")"
        lock_ra_settings="$(config_get_bool "lock_ra_settings" "true")"
        allowed_apps_count="$(config_get_allowed_apps_count)"
        if [ "$hide_games" = "true" ]; then
            hide_label="Hide Games: ON"
        else
            hide_label="Hide Games: OFF"
        fi
        if [ "$lock_ra_settings" = "true" ]; then
            lock_ra_label="Lock RA Settings: ON"
        else
            lock_ra_label="Lock RA Settings: OFF"
        fi
        allowed_apps_label="Allowed Apps: $allowed_apps_count"

        run_prompt -t "Kids Mode [OFF]" -m "Choose an action." \
            "Lock now" \
            "Set PIN" \
            "$hide_label" \
            "$lock_ra_label" \
            "$allowed_apps_label" \
            "Recover" \
            "Status" \
            "Cancel"
        action=$?

        case "$action" in
            0)
                if with_lock command_lock; then
                    show_info "Kids Mode" "Kids Mode is now locked."
                else
                    show_info "Kids Mode" "Lock failed. Check logs in .KidMode/kidmode.log"
                fi
                ;;
            1)
                command_set_pin_interactive || true
                ;;
            2)
                command_toggle_hide_games
                ;;
            3)
                command_toggle_lock_ra_settings
                ;;
            4)
                command_manage_allowed_apps || true
                ;;
            5)
                with_lock command_recover || true
                ;;
            6)
                status_lines="$(command_status | sed ':a;N;$!ba;s/\n/\\n/g')"
                show_info "Kids Mode Status" "$status_lines"
                ;;
            *)
                return 0
                ;;
        esac
    done
}

main() {
    cmd="${1:-manager}"

    case "$cmd" in
        manager)
            command_manager_menu
            ;;
        lock)
            with_lock command_lock
            ;;
        unlock)
            ensure_layout
            shift

            if [ "${1:-}" = "--pin" ] && [ -n "${2:-}" ]; then
                pin="$2"
                if ! verify_pin "$pin"; then
                    echo "Kid Mode: invalid PIN" >&2
                    exit 1
                fi
                with_lock command_unlock
                exit $?
            fi

            echo "Kid Mode: unlock requires --pin #### or use unlock-interactive" >&2
            exit 1
            ;;
        unlock-interactive)
            command_unlock_interactive
            ;;
        recover)
            with_lock command_recover
            ;;
        status)
            command_status
            ;;
        set-pin)
            ensure_layout
            shift
            if [ -z "${1:-}" ]; then
                echo "Kid Mode: set-pin requires a 4-digit argument" >&2
                exit 1
            fi
            set_pin_value "$1"
            ;;
        set-pin-interactive)
            command_set_pin_interactive
            ;;
        toggle-hide-games)
            command_toggle_hide_games
            ;;
        toggle-lock-ra-settings)
            command_toggle_lock_ra_settings
            ;;
        manage-allowed-apps)
            command_manage_allowed_apps
            ;;
        *)
            usage
            exit 1
            ;;
    esac
}

main "$@"
