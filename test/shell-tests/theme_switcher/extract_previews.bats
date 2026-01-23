#!/usr/bin/env bats

load '../support/test_helper'
load '../support/mocks'
load 'theme_helper'

setup() {
    setup_sdcard
    cleanup_test_data
    mock_system_commands
    mock_md5sum
    
    mkdir -p "$SDCARD/Themes"
}

@test "themes_extract_previews.sh extracts new previews" {
    create_theme_archive "NewTheme" "$SDCARD/Themes"
    
    run sh "$PROJECT/src/themeSwitcher/script/themes_extract_previews.sh"
    
    assert_success
    assert_file_exists "$SDCARD/Themes/.previews/NewTheme/config.json"
    assert_file_exists "$SDCARD/Themes/.previews/NewTheme/md5hash"
    assert_file_exists "$SDCARD/Themes/.previews/NewTheme/source"
    assert_file_contains "$SDCARD/Themes/.previews/NewTheme/source" "NewTheme.7z"
}

@test "themes_extract_previews.sh skips if theme already installed" {
    create_theme_archive "ExistingTheme" "$SDCARD/Themes"
    
    # Mark as installed
    mkdir -p "$SDCARD/Themes/ExistingTheme"
    echo "abc123def456" > "$SDCARD/Themes/ExistingTheme/md5hash"
    
    run sh "$PROJECT/src/themeSwitcher/script/themes_extract_previews.sh"
    
    assert_success
    assert_file_not_exists "$SDCARD/Themes/ExistingTheme.7z"
    assert_output --partial "[IGNORE] theme already installed: ExistingTheme"
}

@test "themes_extract_previews.sh skips if preview already exists" {
    create_theme_archive "PreviewedTheme" "$SDCARD/Themes"

    # Mark as previewed
    mkdir -p "$SDCARD/Themes/.previews/PreviewedTheme"
    echo "abc123def456" > "$SDCARD/Themes/.previews/PreviewedTheme/md5hash"
    
    run sh "$PROJECT/src/themeSwitcher/script/themes_extract_previews.sh"
    
    assert_success
    assert_output --partial "[IGNORE] found preview for: PreviewedTheme"
}
