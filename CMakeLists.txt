cmake_minimum_required(VERSION 3.20)
project(redundant_csp VERSION 1.0.0 DESCRIPTION "Redundant CSP")

find_package(Threads REQUIRED)

add_subdirectory(libcsp)
add_subdirectory(server)
add_subdirectory(client)
