# Guest Mode test fixtures and setup

setup_guest_mode() {
    setup_sdcard
    
    mkdir -p "$SDCARD/Saves/CurrentProfile/theme"
    mkdir -p "$SDCARD/Saves/CurrentProfile/lists"
    mkdir -p "$SDCARD/Saves/CurrentProfile/saves"
    mkdir -p "$SDCARD/Roms"
    mkdir -p "$SDCARD/Themes/Silky by DiMo"
    mkdir -p "$SDCARD/.tmp_update/config/startup"
    
    cat > "$SDCARD/system.json" << 'EOF'
{
  "theme": "/mnt/SDCARD/Themes/Silky by DiMo/"
}
EOF
}

setup_guest_mode_scripts() {
    local script_dir="$SDCARD/App/Guest_Mode"
    local src_dir="$PROJECT/static/packages/App/Guest Mode/App/Guest_Mode"
    mkdir -p "$script_dir/data"
    
    if [ -d "$src_dir" ]; then
        cp "$src_dir/"*.sh "$script_dir/" 2>/dev/null || true
        cp "$src_dir/data/"*.json "$script_dir/data/" 2>/dev/null || true
        chmod +x "$script_dir/"*.sh 2>/dev/null || true
    fi
}
