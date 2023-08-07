#!/bin/sh

# The JSON data
json_regions=$1
# The region name to search for
region=$(echo "$2" | tr '[:upper:]' '[:lower:]')

# Function to recursively search for parent regions and return their nomcourt values
find_parents() {
    id=$1
    parents=""
    while true; do
        # Find the region with the matching id
        region=$(echo "$json_regions" | jq -r --arg id "$id" '.regions.region[] | select(.id == ($id | tonumber))')
        # If no region is found, break the loop
        if [ -z "$region" -o "$region" = "null" ]; then
            break
        fi
        # Add the nomcourt value of the region to the parents list
        parents="$parents $(echo "$region" | jq -r '.nomcourt')"
        # Get the id of the parent region
        id=$(echo "$region" | jq '.parent')
    done
    echo $parents
}

# Find the region with the matching name
region=$(echo "$json_regions" | jq -r --arg region "$region" '.regions.region[] | select((.nomcourt | ascii_downcase) == $region)')

# If no region is found, print an error message and exit
if [ -z "$region" -o "$region" = "null" ]; then
    echo "No region found that matches the input name."
    return 1
fi

# Get the id of the parent region
parent_id=$(echo "$region" | jq '.parent')

# Find and print the parent regions
parents=$(find_parents "$parent_id")

# Print the prioritized list of nomcourt values
echo "$2 $parents fallback"