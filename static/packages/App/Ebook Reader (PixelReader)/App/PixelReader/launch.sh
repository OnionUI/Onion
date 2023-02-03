#!/bin/sh
cd $(dirname "$0")

export LD_LIBRARY_PATH=$(dirname "$0")/lib:$LD_LIBRARY_PATH
./reader 2>log.txt
