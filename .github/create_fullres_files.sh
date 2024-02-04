#!/bin/bash

# Creates full_resolution files where appropriate
# If this file is present, the program will be launched in 560p by runtime.sh

# Include hidden folders
shopt -s dotglob

packages="./build/App/PackageManager/data"

# DraStic
touch "$packages/Emu/Nintendo - DS (Drastic)/Emu/NDS/full_resolution"
