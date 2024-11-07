#!/bin/bash

set -e

run_test() {
    ./build/server/server -f ./build/server/server_virt.yaml &
    ./build/client/client -f ./build/client/client_virt.yaml -c 10
}

run_test

sudo ifconfig vcan0 down
run_test
sudo ifconfig vcan0 up

sudo ifconfig vcan1 down
run_test
sudo ifconfig vcan1 up
