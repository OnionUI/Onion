# Core test helper - loads BATS libraries and provides base setup

# Load BATS helper libraries
load '/opt/bats-support/load'
load '/opt/bats-assert/load'
load '/opt/bats-file/load'

# Common paths
export SDCARD="/mnt/SDCARD"
export TEST_DATA="/tmp/test_data"
export PROJECT="/root/workspace"
export MOCK_LOG="/tmp/mock_calls.log"

# Reset mock SDCARD to clean state
setup_sdcard() {
    rm -rf "$SDCARD"
    mkdir -p "$SDCARD/App" \
             "$SDCARD/Emu" \
             "$SDCARD/RApp" \
             "$SDCARD/.tmp_update/bin" \
             "$SDCARD/.tmp_update/config" \
             "$SDCARD/Saves/CurrentProfile"
    mkdir -p "$TEST_DATA"
}

# Clean up test data
cleanup_test_data() {
    rm -rf "$TEST_DATA"
    mkdir -p "$TEST_DATA"
    rm -f "$MOCK_LOG"
}
