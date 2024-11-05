cmake_minimum_required(VERSION 3.20)
project(server VERSION 1.0.0 DESCRIPTION "Redundant CSP server")

add_executable(
    ${PROJECT_NAME}
    "src/main.c"
)

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC csp)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/conf/server_0.yaml server_0.yaml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/conf/server_1.yaml server_1.yaml COPYONLY)
