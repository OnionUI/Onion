#!/usr/bin/env bats

load '../../support/test_helper'
load '../../support/mocks'

setup() {
    setup_sdcard
    cleanup_test_data
    mock_system_commands
    
    # Create mock source directory
    # apply.sh expects a folder named the same as the target's basename in its own directory
    mkdir -p "$TEST_DATA/common/App"
    touch "$TEST_DATA/common/App/common_script.sh"
    cp "$PROJECT/static/packages/common/apply.sh" "$TEST_DATA/common/apply.sh"
    
    # Create target structure on SDCARD
    mkdir -p "$SDCARD/App/Package1"
    mkdir -p "$SDCARD/App/Package2"
    mkdir -p "$SDCARD/App/NoConfig"
    
    echo '{"name": "p1"}' > "$SDCARD/App/Package1/config.json"
    echo '{"name": "p2"}' > "$SDCARD/App/Package2/config.json"
}

@test "apply.sh copies scripts to directories with config.json" {
    run sh "$TEST_DATA/common/apply.sh" "$SDCARD/App"
    
    assert_success
    assert_file_exists "$SDCARD/App/Package1/common_script.sh"
    assert_file_exists "$SDCARD/App/Package2/common_script.sh"
}

@test "apply.sh skips directories without config.json" {
    run sh "$TEST_DATA/common/apply.sh" "$SDCARD/App"
    
    assert_success
    assert_file_not_exists "$SDCARD/App/NoConfig/common_script.sh"
}

@test "apply.sh exits gracefully if source directory missing" {
    rm -rf "$TEST_DATA/common/App"
    
    run sh "$TEST_DATA/common/apply.sh" "$SDCARD/App"
    
    assert_success # Script just exits, no error code
    assert_file_not_exists "$SDCARD/App/Package1/common_script.sh"
}
