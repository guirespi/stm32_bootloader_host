set(lib_name "ezm_api_log")

set(${lib_name}_srcs
    "${CMAKE_SOURCE_DIR}/../${lib_name}/src/ezm_api_log.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../${lib_name}/arch/sl/siwg917x/ezm_log_arch_siwg917x.c"
)

set(${lib_name}_public_inc_dir
    "${CMAKE_SOURCE_DIR}/../${lib_name}/inc"
)

set(${lib_name}_priv_inc_dir
    "${CMAKE_SOURCE_DIR}/../${lib_name}/arch/common"
)