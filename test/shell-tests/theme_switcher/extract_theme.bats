#!/usr/bin/env bats

load '../support/test_helper'
load '../support/mocks'
load 'theme_helper'

setup() {
    setup_sdcard
    cleanup_test_data
    mock_system_commands
    mock_md5sum
    
    # Setup test directories
    mkdir -p "$SDCARD/Themes"
}

teardown() {
    rm -f "/tmp/remove_theme_archive"
}

@test "themes_extract_theme.sh extracts theme and generates md5" {
    create_theme_archive "GenericTheme" "$TEST_DATA/theme_src" "icons/icon.png"

    # Script expects preview_dir as $1 and looks for $1/source
    mkdir -p "$SDCARD/Themes/.previews/GenericTheme"
    echo "$TEST_DATA/theme_src/GenericTheme.7z" > "$SDCARD/Themes/.previews/GenericTheme/source"
    
    run sh "$PROJECT/src/themeSwitcher/script/themes_extract_theme.sh" \
        "$SDCARD/Themes/.previews/GenericTheme"
    
    assert_success
    assert_file_exists "$SDCARD/Themes/GenericTheme/md5hash"
    assert_file_contains "$SDCARD/Themes/GenericTheme/md5hash" "abc123def456"
    assert_file_exists "$SDCARD/Themes/GenericTheme/icons/icon.png"
}

@test "themes_extract_theme.sh removes archive if flag set" {
    create_theme_archive "GenericTheme" "$TEST_DATA/theme_src"

    touch "/tmp/remove_theme_archive"
    mkdir -p "$SDCARD/Themes/.previews/GenericTheme"
    echo "$TEST_DATA/theme_src/GenericTheme.7z" > "$SDCARD/Themes/.previews/GenericTheme/source"
    
    # We need the output dir to exist for the script to continue after extraction
    mkdir -p "$SDCARD/Themes/GenericTheme"
    
    run sh "$PROJECT/src/themeSwitcher/script/themes_extract_theme.sh" \
        "$SDCARD/Themes/.previews/GenericTheme"
    
    assert_success
    assert_file_not_exists "$TEST_DATA/theme_src/GenericTheme.7z"
}

@test "themes_extract_theme.sh preserves existing icons if archive has no config.json at depth" {
    # Create archive manually here because we need a specific non-standard structure (no config at root)
    mkdir -p "$TEST_DATA/no_config/icons"
    touch "$TEST_DATA/no_config/icons/new_icon.png"
    cd "$TEST_DATA"
    7z a NoConfigTheme.7z no_config/ >/dev/null
    
    mkdir -p "$SDCARD/Themes/.previews/NoConfigTheme"
    echo "$TEST_DATA/NoConfigTheme.7z" > "$SDCARD/Themes/.previews/NoConfigTheme/source"
    
    mkdir -p "$SDCARD/Themes/NoConfigTheme/icons"
    touch "$SDCARD/Themes/NoConfigTheme/icons/old_icon.png"
    
    run sh "$PROJECT/src/themeSwitcher/script/themes_extract_theme.sh" \
        "$SDCARD/Themes/.previews/NoConfigTheme"
    
    assert_success
    assert_file_exists "$SDCARD/Themes/NoConfigTheme/icons/old_icon.png"
}
