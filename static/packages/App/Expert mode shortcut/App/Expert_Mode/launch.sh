#!/bin/sh

write_state() {
    echo "{\"list\":[{\"title\":132,\"type\":0,\"currpos\":0,\"pagestart\":0,\"pageend\":3},{\"title\":$1,\"type\":$2,\"currpos\":0,\"pagestart\":0,\"pageend\":$3}]}" > /tmp/state.json
}

write_state 0 16 5 # expert tab
