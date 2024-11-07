#!/bin/bash

set -e

SERVER_BUILD_DIR=./build/server/
SERVER_BIN=server
SERVER_CONF_FILE=server_virt_addr2.yaml

CLIENT_BUILD_DIR=./build/client/
CLIENT_BIN=client
CLIENT_CONF_FILE=client_virt.yaml

run_test() {
    ${SERVER_BUILD_DIR}${SERVER_BIN} -f ${SERVER_BUILD_DIR}${SERVER_CONF_FILE} &
    ${CLIENT_BUILD_DIR}${CLIENT_BIN} -f ${CLIENT_BUILD_DIR}${CLIENT_CONF_FILE} -c $1 -s 1 -a 2
}

# Both busses up
run_test $1

# VCAN0 bus down
sudo ifconfig vcan0 down
run_test $1
sudo ifconfig vcan0 up

# VCAN1 bus down
sudo ifconfig vcan1 down
run_test $1
sudo ifconfig vcan1 up

# Run test
${SERVER_BUILD_DIR}${SERVER_BIN} -f ${SERVER_BUILD_DIR}${SERVER_CONF_FILE} &
${CLIENT_BUILD_DIR}${CLIENT_BIN} -f ${CLIENT_BUILD_DIR}${CLIENT_CONF_FILE} -c 100000 -s 1 -a 2 &

# Alternate busses state
sudo ifconfig vcan1 down
sleep 1
sudo ifconfig vcan1 up
sleep 1
sudo ifconfig vcan0 down
sleep 1
sudo ifconfig vcan0 up
