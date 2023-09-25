#!/bin/sh

cd /mnt/SDCARD/App
rm -rf \
    EasyNetplay/ \
    EasyNetplayPokemon \
    2> /dev/null

cd /mnt/SDCARD/App/romscripts
rm -f \
    "./netplay_client.sh" \
    "./netplay_server.sh" \
    "./pokemon_client.sh" \
    "./pokemon_server.sh" \
    2> /dev/null
