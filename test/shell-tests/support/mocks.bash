# Mock utilities for shell script testing

# Create a mock executable that logs usage
mock_binary() {
    local name="$1"
    local bin_path="/usr/local/bin/$name"
    
    export PATH="/usr/local/bin:$PATH"
    
    echo '#!/bin/sh' > "$bin_path"
    echo "echo \"called $name with: \$*\" >> $MOCK_LOG" >> "$bin_path"
    echo "exit 0" >> "$bin_path"
    chmod +x "$bin_path"
}

# Mock standard system commands for performance/safety
mock_system_commands() {
    eval "sync() { :; }"
    export -f sync
    
    eval "sleep() { :; }"
    export -f sleep
    
    eval "reboot() { echo 'reboot called' >> $MOCK_LOG; }"
    export -f reboot
    
    eval "poweroff() { echo 'poweroff called' >> $MOCK_LOG; }"
    export -f poweroff
}

# Mock date command to return predictable values
mock_date() {
    local timestamp="${1:-1700000000}"
    local bin_path="/usr/local/bin/date"
    
    export PATH="/usr/local/bin:$PATH"
    
    cat > "$bin_path" << 'MOCK_EOF'
#!/bin/sh
if [ "$1" = "+%s" ]; then
    if [ "$2" = "-s" ]; then
        echo "date set to: $3" >> /tmp/mock_calls.log
        exit 0
    fi
    echo "MOCK_TIMESTAMP"
    exit 0
fi
/bin/date "$@"
MOCK_EOF
    sed -i "s/MOCK_TIMESTAMP/$timestamp/" "$bin_path"
    chmod +x "$bin_path"
}

# Mock themeSwitcher binary
mock_themeSwitcher() {
    mock_binary "themeSwitcher"
}

# Mock md5sum to return predictable hash
# Usage: mock_md5sum [hash]
mock_md5sum() {
    local hash="${1:-abc123def456}"
    local bin_path="/usr/local/bin/md5sum"
    export PATH="/usr/local/bin:$PATH"
    
    echo '#!/bin/sh' > "$bin_path"
    echo "echo \"$hash  \$1\"" >> "$bin_path"
    chmod +x "$bin_path"
}
