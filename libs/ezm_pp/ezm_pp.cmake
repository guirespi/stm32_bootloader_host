set(lib_name "ezm_pp")

set(${lib_name}_srcs
    "${CMAKE_SOURCE_DIR}/../${lib_name}/src/ezm_pp_drv.c"
    "${CMAKE_SOURCE_DIR}/../${lib_name}/src/ezm_pp_ep_drv.c"
    "${CMAKE_SOURCE_DIR}/../${lib_name}/src/ezm_pp_obj.c"
    "${CMAKE_SOURCE_DIR}/../${lib_name}/src/ezm_pp_pkt_maker.c"
    "${CMAKE_SOURCE_DIR}/../${lib_name}/src/ezm_pp_pkt_processor.c"
    "${CMAKE_SOURCE_DIR}/../${lib_name}/src/ezm_pp_uart_drv.c"
    "${CMAKE_SOURCE_DIR}/../${lib_name}/src/ezm_pp_main.c"
)

set(${lib_name}_public_inc_dir
    "${CMAKE_SOURCE_DIR}/../${lib_name}/inc"
)