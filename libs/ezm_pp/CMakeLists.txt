cmake_minimum_required(VERSION 3.22)
set(lib_name ezm_pp)
set(CMAKE_C_STANDARD 11)

project(
    ${lib_name}
    VERSION 1.0.0
    DESCRIPTION "Production Protocol"
)

set(srcs 
    "src/ezm_pp_drv.c"
    "src/ezm_pp_ep_drv.c"
    "src/ezm_pp_obj.c"
    "src/ezm_pp_pkt_maker.c"
    "src/ezm_pp_pkt_processor.c"
    "src/ezm_pp_uart_drv.c"
    "src/ezm_pp_main.c"
)

set(public_inc_dir
    "inc"
)

set(private_inc_dir
)

add_library(
    ${lib_name}
    STATIC
    ${srcs}
)

target_include_directories(
    ${lib_name}
    PUBLIC
    ${public_inc_dir}
    PRIVATE
    ${private_inc_dir}
)

target_link_libraries(
    ${lib_name}
    ezm_api_log
    ezm_utility
    ezm_api_console
)

file(GLOB public_inc_files "${public_inc_dir}/*.h")
set_target_properties(
    ${lib_name} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "${public_inc_files}"
)