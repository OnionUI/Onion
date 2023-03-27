#!/bin/sh
echo $0 $*

set -o pipefail

./random.sh

if [ $? -eq 0 ]; then
    touch /tmp/quick_switch
fi
