cmake_minimum_required(VERSION 3.20)
project(gateway VERSION 1.0.0 DESCRIPTION "CSP gateway")

add_executable(
    ${PROJECT_NAME}
    "src/main.c"
)

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC utils)
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC csp)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/conf/gateway_virt.yaml gateway_virt.yaml COPYONLY)
