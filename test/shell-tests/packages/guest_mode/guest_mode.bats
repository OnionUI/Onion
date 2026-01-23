#!/usr/bin/env bats

load '../../support/test_helper'
load '../../support/mocks'
load 'guest_mode_helper'

setup() {
    cleanup_test_data
    mock_system_commands
    setup_guest_mode
    setup_guest_mode_scripts
    mock_themeSwitcher
}

# install.sh tests

@test "Guest Mode: install.sh sets configON when MainProfile exists" {
    mkdir -p "$SDCARD/Saves/MainProfile"
    
    cd "$SDCARD/App/Guest_Mode"
    run sh ./install.sh
    
    assert_success
    run diff ./config.json ./data/configON.json
    assert_success
}

@test "Guest Mode: install.sh does nothing when MainProfile missing" {
    echo '{"original": true}' > "$SDCARD/App/Guest_Mode/config.json"
    
    cd "$SDCARD/App/Guest_Mode"
    run sh ./install.sh
    
    assert_success
    assert_file_contains "$SDCARD/App/Guest_Mode/config.json" '"original": true'
}

# saveTime.sh tests

@test "Guest Mode: saveTime.sh saves timestamp to currentTime.txt" {
    mock_date "1700000000"
    
    cd "$SDCARD/App/Guest_Mode"
    run sh ./saveTime.sh
    
    assert_success
    assert_file_exists "$SDCARD/Saves/CurrentProfile/saves/currentTime.txt"
    assert_file_contains "$SDCARD/Saves/CurrentProfile/saves/currentTime.txt" "1700000000"
}

# loadTime.sh tests

@test "Guest Mode: loadTime.sh adds default 4-hour offset" {
    mock_date "1700000000"
    echo "1700000000" > "$SDCARD/Saves/CurrentProfile/saves/currentTime.txt"
    
    cd "$SDCARD/App/Guest_Mode"
    run sh ./loadTime.sh
    
    assert_success
    assert_file_contains "$MOCK_LOG" "date set to: @1700014400"
}

@test "Guest Mode: loadTime.sh uses custom hours offset" {
    mock_date "1700000000"
    echo "1700000000" > "$SDCARD/Saves/CurrentProfile/saves/currentTime.txt"
    echo "2" > "$SDCARD/.tmp_update/config/startup/addHours"
    
    cd "$SDCARD/App/Guest_Mode"
    run sh ./loadTime.sh
    
    assert_success
    assert_file_contains "$MOCK_LOG" "date set to: @1700007200"
}

@test "Guest Mode: loadTime.sh skips offset when NTP enabled" {
    mock_date "1700000000"
    echo "1700000000" > "$SDCARD/Saves/CurrentProfile/saves/currentTime.txt"
    touch "$SDCARD/.tmp_update/config/.ntpState"
    
    cd "$SDCARD/App/Guest_Mode"
    run sh ./loadTime.sh
    
    assert_success
    assert_file_contains "$MOCK_LOG" "date set to: @1700000000"
}

@test "Guest Mode: loadTime.sh handles missing currentTime.txt" {
    mock_date "1700000000"
    rm -f "$SDCARD/Saves/CurrentProfile/saves/currentTime.txt"
    
    cd "$SDCARD/App/Guest_Mode"
    run sh ./loadTime.sh
    
    assert_success
    assert_file_contains "$MOCK_LOG" "date set to: @14400"
}

# launch.sh tests

@test "Guest Mode: launch.sh switches Main to Guest" {
    mkdir -p "$SDCARD/Saves/GuestProfile/theme"
    mkdir -p "$SDCARD/Saves/GuestProfile/lists"
    echo "/mnt/SDCARD/Themes/GuestTheme/" > "$SDCARD/Saves/GuestProfile/theme/currentTheme"
    echo '{"guest": "favorites"}' > "$SDCARD/Saves/GuestProfile/lists/favorites.json"
    
    echo "/mnt/SDCARD/Themes/MainTheme/" > "$SDCARD/Saves/CurrentProfile/theme/currentTheme"
    echo '{"main": "recents"}' > "$SDCARD/Roms/recentlist.json"
    
    mock_date "1700000000"
    
    cd "$SDCARD/App/Guest_Mode"
    run sh ./launch.sh
    
    assert_success
    assert_dir_exists "$SDCARD/Saves/MainProfile"
    assert_dir_not_exists "$SDCARD/Saves/GuestProfile"
}

@test "Guest Mode: launch.sh switches Guest to Main" {
    mkdir -p "$SDCARD/Saves/MainProfile/theme"
    mkdir -p "$SDCARD/Saves/MainProfile/lists"
    echo "/mnt/SDCARD/Themes/MainTheme/" > "$SDCARD/Saves/MainProfile/theme/currentTheme"
    
    mock_date "1700000000"
    
    cd "$SDCARD/App/Guest_Mode"
    run sh ./launch.sh
    
    assert_success
    assert_dir_exists "$SDCARD/Saves/GuestProfile"
    assert_dir_not_exists "$SDCARD/Saves/MainProfile"
}

@test "Guest Mode: launch.sh calls themeSwitcher" {
    mkdir -p "$SDCARD/Saves/GuestProfile"
    mock_date "1700000000"
    
    cd "$SDCARD/App/Guest_Mode"
    run sh ./launch.sh
    
    assert_success
    assert_file_contains "$MOCK_LOG" "called themeSwitcher with: --reapply_icons"
}

@test "Guest Mode: launch.sh preserves theme via system.json" {
    mkdir -p "$SDCARD/Saves/GuestProfile/theme"
    echo "/mnt/SDCARD/Themes/GuestTheme/" > "$SDCARD/Saves/GuestProfile/theme/currentTheme"
    
    mock_date "1700000000"
    
    cd "$SDCARD/App/Guest_Mode"
    run sh ./launch.sh
    
    assert_success
    run jq -r .theme "$SDCARD/system.json"
    assert_output "/mnt/SDCARD/Themes/GuestTheme/"
}
