set(lib_name "ezm_api_console")

set(${lib_name}_srcs
    "${CMAKE_SOURCE_DIR}/../${lib_name}/src/ezm_api_console.c"
    "${CMAKE_SOURCE_DIR}/../${lib_name}/arch/sl/siwg917x/ezm_console_arch_siwg917x.c"
)

set(${lib_name}_public_inc_dir
    "${CMAKE_SOURCE_DIR}/../${lib_name}/inc"
)

set(${lib_name}_priv_inc_dir
    "${CMAKE_SOURCE_DIR}/../${lib_name}/arch/common"
)
