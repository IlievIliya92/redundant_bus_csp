cmake_minimum_required(VERSION 3.20)
project(server_csp VERSION 1.0.0 DESCRIPTION "Redundant CSP server")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/conf/module_0_virt.yaml module_0_virt.yaml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/conf/module_0_rtable_virt.txt module_0_rtable_virt.txt COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/conf/module_1_virt.yaml module_1_virt.yaml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/conf/module_1_rtable_virt.txt module_1_rtable_virt.txt COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/conf/module_2_virt.yaml module_2_virt.yaml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/conf/module_2_rtable_virt.txt module_2_rtable_virt.txt COPYONLY)

add_executable(
    ${PROJECT_NAME}
    "src/main.c"
)

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC utils)
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC csp)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/local/bin)
