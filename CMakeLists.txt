cmake_minimum_required(VERSION 3.22)

set(srcs 
    "src/app_bootloader_host.c"
    "src/app_bootloader_command.c"
)

set(
    public_inc_dir 
    "inc"
)

set(
    private_inc_dir
    ""
)

set(CMAKE_C_STANDARD 11)

# Add libs subdirectory 
add_subdirectory(libs)

set(lib_name "app_bootloader_host")

project(
    ${lib_name}
    VERSION 1.0.0
    DESCRIPTION "Bootloader application for host"    
)

add_executable(${lib_name} ${srcs})
target_include_directories(
    ${lib_name}
    PUBLIC
    ${public_inc_dir}
    PRIVATE
    ${private_inc_dir}    
)

target_link_libraries(
    ${lib_name}
    api_log
)
