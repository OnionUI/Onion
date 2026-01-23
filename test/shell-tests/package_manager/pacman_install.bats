#!/usr/bin/env bats

load '../support/test_helper'
load '../support/mocks'

setup() {
    setup_sdcard
    cleanup_test_data
    mock_system_commands
    mkdir -p "$TEST_DATA/TestPackage/App/TestApp"
}

@test "pacman_install.sh copies package files to SDCARD" {
    echo '{"label": "New App"}' > "$TEST_DATA/TestPackage/App/TestApp/config.json"
    
    run sh "$PROJECT/src/packageManager/script/pacman_install.sh" "$TEST_DATA" TestPackage
    
    assert_success
    assert_file_exists "$SDCARD/App/TestApp/config.json"
}

@test "pacman_install.sh preserves existing label and imgpath" {
    # IMPORTANT: Use multi-line JSON - script parser is fragile with single-line
    cat > "$TEST_DATA/TestPackage/App/TestApp/config.json" <<EOF
{
  "label": "Default",
  "imgpath": "/default.png"
}
EOF
    
    mkdir -p "$SDCARD/App/TestApp"
    cat > "$SDCARD/App/TestApp/config.json" <<EOF
{
  "label": "My Custom Name",
  "imgpath": "/my/icon.png"
}
EOF
    
    run sh "$PROJECT/src/packageManager/script/pacman_install.sh" "$TEST_DATA" TestPackage
    
    assert_success
    assert_file_contains "$SDCARD/App/TestApp/config.json" '"label": "My Custom Name"'
    assert_file_contains "$SDCARD/App/TestApp/config.json" '"imgpath": "/my/icon.png"'
}

@test "pacman_install.sh handles missing existing config gracefully" {
    echo '{"label": "Fresh Install"}' > "$TEST_DATA/TestPackage/App/TestApp/config.json"
    
    run sh "$PROJECT/src/packageManager/script/pacman_install.sh" "$TEST_DATA" TestPackage
    
    assert_success
    assert_file_contains "$SDCARD/App/TestApp/config.json" '"label": "Fresh Install"'
}
