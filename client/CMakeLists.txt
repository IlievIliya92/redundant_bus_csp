cmake_minimum_required(VERSION 3.20)
project(client_csp VERSION 1.0.0 DESCRIPTION "Redundant CSP client")

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH True)

add_executable(
    ${PROJECT_NAME}
    "src/main.c"
)

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC utils)
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC csp)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/conf/client_virt.yaml client_virt.yaml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/conf/gs_virt.yaml gs_virt.yaml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/conf/gs_rtable_virt.txt gs_rtable_virt.txt COPYONLY)

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/local/bin)