#!/bin/sh

# Check if the correct number of arguments are provided
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <path_to_regions.json> <search_term>"
    exit 1
fi

# Assign arguments to variables
regions_file="$1"
search_term="$2"

# Convert search term to lowercase (for case-insensitive search)
search_term=$(echo "$search_term" | tr '[:upper:]' '[:lower:]')

# This will be the composite list of regions
composite_region_list=""

# Function to find the region and its parents
find_region_by_name() {
    local region_name="$1"
    local region=$(jq -r --arg name "$region_name" 'if .Data.regions and .Data.regions.region then .Data.regions.region[] | select((if .nomcourt? then (.nomcourt | ascii_downcase) else empty end) == $name or (if .nom_en? then (.nom_en | ascii_downcase) else empty end) == $name or (if .nom_fr? then (.nom_fr | ascii_downcase) else empty end) == $name or (if .nom_es? then (.nom_es | ascii_downcase) else empty end) == $name or (if .nom_de? then (.nom_de | ascii_downcase) else empty end) == $name or (if .nom_it? then (.nom_it | ascii_downcase) else empty end) == $name or (if .nom_pt? then (.nom_pt | ascii_downcase) else empty end) == $name) else empty end' "$regions_file")
    
    if [ ! -z "$region" ]; then
        composite_region_list="${composite_region_list} $(echo "$region" | jq -r '.nomcourt')"
        if [ $(echo "$region" | jq -r '.parent') != "null" ]; then
            find_region_by_id $(echo "$region" | jq -r '.parent')
        fi
    fi
}

find_region_by_id() {
    local region_id="$1"
    local region=$(jq -r --arg id "$region_id" 'if .Data.regions and .Data.regions.region then .Data.regions.region[] | select(.id == ($id | tonumber)) else empty end' "$regions_file")
    
    if [ ! -z "$region" ]; then
        composite_region_list="${composite_region_list} $(echo "$region" | jq -r '.nomcourt')"
        if [ $(echo "$region" | jq -r '.parent') != "null" ]; then
            find_region_by_id $(echo "$region" | jq -r '.parent')
        fi
    fi
}

# Start the search by name
find_region_by_name "$search_term"

# Print the composite region list
echo "$composite_region_list"