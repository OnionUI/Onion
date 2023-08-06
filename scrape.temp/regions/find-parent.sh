#!/bin/sh

if [ "$#" -ne 2 ]; then
  echo "Usage: $0 <file_path> <value>"
  exit 1
fi

file_path="$1"
value="$(echo "$2" | tr '[:upper:]' '[:lower:]')"

# Function to find parents recursively
find_parents() {
    region_id="$1"
    nomcourt=$(jq -r --argjson region_id "$region_id" '.regions[] | select(.id == $region_id) | .nomcourt' "$file_path")
    parent_id=$(jq -r --argjson region_id "$region_id" '.regions[] | select(.id == $region_id) | .parent' "$file_path")

    if [ "$parent_id" -ne 0 ]; then
        find_parents "$parent_id"
    fi

    echo "$nomcourt"
}

region_id=""
if [ -z "$region_id" ]; then
    # Search using nomcourt
    region_id=$(jq -r --arg value "$value" '.regions[] | select(.nomcourt | ascii_downcase == $value) | .id' "$file_path")

    # If not found using nomcourt, search using nom_en
    if [ -z "$region_id" ]; then
        region_id=$(jq -r --arg value "$value" '.regions[] | select(.nom_en | ascii_downcase == $value) | .id' "$file_path")
    fi
fi

if [ -n "$region_id" ]; then
    echo "Searching for path from $2 to World (ID 0):"
    find_parents "$region_id" | tac
else
    echo "Region not found."
fi
