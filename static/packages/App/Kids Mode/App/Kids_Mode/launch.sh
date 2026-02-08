#!/bin/sh

progdir="$(CDPATH= cd -- "$(dirname "$0")" >/dev/null 2>&1 && pwd -P)"
cd "$progdir" || exit 1

./kidmodectl.sh manager
