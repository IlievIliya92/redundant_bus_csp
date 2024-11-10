# Redundant CSP CAN bus

This repository contains a project that implements a **redundant CAN bus communication system** using the **Cubesat Space Protocol (CSP)**. The goal of the project is to ensure reliable communication between nodes in a distributed system by leveraging redundancy in the CAN bus network. The project is structured into several components, including clients, servers, and utilities, and is built using **libcsp**.

## Overview
The **Redundant CSP CAN Bus** project provides a fault-tolerant communication protocol over a CAN bus network by implementing redundancy mechanisms. This ensures that the system can continue to operate even in the event of failures in parts of the network.

The project uses **CSP (Cubesat Space Protocol)**, a lightweight protocol designed for small satellite systems but applicable to any distributed embedded systems.

## Repository Structure
The repository is organized as follows:

```
redundant_bus_csp/
│
├── .github/ # GitHub workflows and CI/CD configurations
├── client/ # Client-side code for communicating over CSP
├── docs/ # Documentation files
├── gateway/ # Gateway implementation for handling redundant paths
├── libcsp/ # Submodule containing the libcsp library
├── scripts/ # Helper scripts testing and running the project
├── server/ # Server-side code for handling requests from clients
├── utils/ # Utility functions and tools used across the project
├── CMakeLists.txt # CMake build configuration file
├── Dockerfile # Docker configuration for containerized builds
└── README.md # This README file
```

## Usage

### Prerequisites
To build and run this project, you will need:
- A C compiler (e.g., GCC)
- CMake (version 3.10 or higher)
- Docker (optional, if you want to use a containerized environment)

### Steps to Build

1. Clone this repository:

```bash
   git clone https://github.com/IlievIliya92/redundant_bus_csp.git
   cd redundant_bus_csp
```

2. Initialize submodules:

```bash
git submodule update --init --recursive
```

3. Build the project using CMake:

```bash
mkdir build && cd build
cmake -DCSP_USE_RTABLE=ON -DCSP_BUFFER_COUNT=50 ..
make
```

4. Optionally, you can use Docker to build and run the project:

```bash
docker build -t redundant_bus_csp .
```
