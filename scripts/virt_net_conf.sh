#!/bin/sh


# Configure virtual can interfaces
sudo ip link add dev vcan0 type vcan
sudo ip link set vcan0 mtu 16
sudo ip link set up vcan0
sudo ip link add dev vcan1 type vcan
sudo ip link set vcan1 mtu 16
sudo ip link set up vcan1

socat -d -d -d pty,raw,echo=0,link=/tmp/ttyUSB0 pty,raw,echo=0,link=/tmp/ttyUSB1 &
