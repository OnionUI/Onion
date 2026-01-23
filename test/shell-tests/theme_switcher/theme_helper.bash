# Helper functions for Theme Switcher tests

# Create a standard theme 7z archive
# Usage: create_theme_archive <theme_name> [output_dir] [extra_files...]
create_theme_archive() {
    local theme_name="$1"
    local output_dir="${2:-$TEST_DATA/theme_src}"
    shift 2

    local src_dir="$TEST_DATA/tmp_src_$theme_name"
    mkdir -p "$src_dir/$theme_name"
    touch "$src_dir/$theme_name/config.json"
    touch "$src_dir/$theme_name/preview.png"
    
    # Create extra files if specified
    # Format: relative/path/to/file
    for file in "$@"; do
        local dir=$(dirname "$src_dir/$theme_name/$file")
        mkdir -p "$dir"
        touch "$src_dir/$theme_name/$file"
    done

    mkdir -p "$output_dir"
    local cwd=$(pwd)
    cd "$src_dir"
    7z a "$output_dir/${theme_name}.7z" "$theme_name/" >/dev/null
    cd "$cwd"
    
    rm -rf "$src_dir"
}
