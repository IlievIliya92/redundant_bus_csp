#!/bin/bash

set -e

SERVER_BUILD_DIR=./build/server/
SERVER_BIN=server

CLIENT_BUILD_DIR=./build/client/
CLIENT_BIN=client
CLIENT_CONF_FILE=client_virt.yaml

${SERVER_BUILD_DIR}${SERVER_BIN} -f ${SERVER_BUILD_DIR}server_virt_addr2.yaml &
${SERVER_BUILD_DIR}${SERVER_BIN} -f ${SERVER_BUILD_DIR}server_virt_addr3.yaml &
${SERVER_BUILD_DIR}${SERVER_BIN} -f ${SERVER_BUILD_DIR}server_virt_addr4.yaml &
${CLIENT_BUILD_DIR}${CLIENT_BIN} -f ${CLIENT_BUILD_DIR}${CLIENT_CONF_FILE} -c $1 -s 1 -a 2 -a 3 -a 4
