#ifndef EZM_PP_PKT_MAKER_H
#define EZM_PP_PKT_MAKER_H

#include "ezm_pp_obj.h"

uint32_t ezm_pp_pkt_maker_req_len(ezm_pp_pkt_t *pkt,
                                  ezm_pp_cmd_code_t cmd_code);

uint32_t ezm_pp_pkt_maker_res_len(ezm_pp_pkt_t *pkt, ezm_pp_cmd_code_t cmd_code,
                                  ezm_pp_cmd_comp_code_t completion_code);

uint32_t ezm_pp_pkt_maker_req(ezm_pp_obj_t * src, ezm_pp_obj_t * dst, ezm_pp_pkt_t *pkt, ezm_pp_cmd_code_t cmd_code);

uint32_t ezm_pp_pkt_maker_res(ezm_pp_obj_t * src, ezm_pp_obj_t * dst, ezm_pp_pkt_t *pkt, ezm_pp_cmd_code_t cmd_code,
                              ezm_pp_cmd_comp_code_t completion_code);

uint32_t ezm_pp_pkt_maker_fill_header(ezm_pp_drv_phy_tp_binding_id_t tp_binding,
                                      void *buffer, uint32_t pp_pkt_len);

ezm_pp_pkt_t *
ezm_pp_pkt_maker_get_pp_pkt_region(ezm_pp_drv_phy_tp_binding_id_t tp_binding,
                                   void *buffer);

// Support for specific commands.
uint32_t ezm_pp_pkt_maker_get_version_req(ezm_pp_obj_t * src, ezm_pp_obj_t * dst, ezm_pp_drv_phy_tp_binding_id_t tp_binding , void * buffer);
uint32_t ezm_pp_pkt_maker_get_storage_info_req(ezm_pp_obj_t * src, ezm_pp_obj_t * dst, ezm_pp_drv_phy_tp_binding_id_t tp_binding , void * buffer);
uint32_t ezm_pp_pkt_maker_get_capabilities_req(ezm_pp_obj_t * src, ezm_pp_obj_t * dst, ezm_pp_drv_phy_tp_binding_id_t tp_binding , void * buffer);
uint32_t ezm_pp_pkt_maker_set_capabilities_req(ezm_pp_obj_t * src, ezm_pp_obj_t * dst, ezm_pp_drv_phy_tp_binding_id_t tp_binding , void * buffer, uint32_t max_data_transfer, uint32_t max_message_size);
uint32_t ezm_pp_pkt_maker_get_img_info_req(ezm_pp_obj_t * src, ezm_pp_obj_t * dst, ezm_pp_drv_phy_tp_binding_id_t tp_binding , void * buffer, uint8_t img_index);
uint32_t ezm_pp_pkt_maker_download_req(ezm_pp_obj_t * src, ezm_pp_obj_t * dst, ezm_pp_drv_phy_tp_binding_id_t tp_binding , void * buffer, uint32_t block_size, uint8_t img_idx, uint32_t img_size);
uint32_t ezm_pp_pkt_maker_boot_img_req(ezm_pp_obj_t * src, ezm_pp_obj_t * dst, ezm_pp_drv_phy_tp_binding_id_t tp_binding , void * buffer, uint8_t img_idx);
uint32_t ezm_pp_pkt_maker_get_block_req(ezm_pp_obj_t * src, ezm_pp_obj_t * dst, ezm_pp_drv_phy_tp_binding_id_t tp_binding , void * buffer, uint32_t block_number);

uint32_t ezm_pp_pkt_maker_get_block_rsp(ezm_pp_obj_t * src, ezm_pp_obj_t * dst, ezm_pp_drv_phy_tp_binding_id_t tp_binding , void * buffer, ezm_pp_cmd_comp_code_t completion_code, uint32_t block_number, uint32_t block_size, uint8_t * block_data);
#endif // EZM_PP_PKT_MAKER_H