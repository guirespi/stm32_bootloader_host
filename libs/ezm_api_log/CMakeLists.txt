cmake_minimum_required(VERSION 3.22)
set(lib_name ezm_api_log)
set(CMAKE_C_STANDARD 11)

project(
    ${lib_name}
    VERSION 1.0.0
    DESCRIPTION "Logger API"
)

set(srcs 
    "src/ezm_api_log.c"
)

if(WIN32)
    list(APPEND srcs
        "arch/os/win/ezm_log_arch_win.c"
    )
endif()


set(public_inc_dir
    "inc"
    "arch/common"
)

set(private_inc_dir
    "arch/common"
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

file(GLOB public_inc_files "${public_inc_dir}/*.h")
set_target_properties(
    ${lib_name} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "${public_inc_files}"
)