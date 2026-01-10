cmake_minimum_required(VERSION 3.22)

project(
    app_production_host
    VERSION 1.0.0
    DESCRIPTION "Production application for host"
)

set(CMAKE_C_STANDARD 11)

set(srcs
    "src/app_production.c"
)

if(WIN32)
    add_compile_definitions(CONFIG_HOST_BUILD)
    add_compile_definitions($<$<CONFIG:Debug>:CONFIG_LOGS_ENABLE>)
endif()

add_subdirectory(libs)

add_executable(app_production_host ${srcs})

message(STATUS "HOST Build is: ${CONFIG_HOST_BUILD}")

target_link_libraries(
    app_production_host
    ezm_api_log
    ezm_utility
    ezm_api_console
    ezm_pp
)