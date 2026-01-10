#ifndef EZM_PP_PKT_H_
#define EZM_PP_PKT_H_

#include "ezm_pp.h"
#include <stddef.h>

#define EZM_PP_MAX_PAYLOAD_SIZE (4096)
#define EZM_PP_MAX_BLOCK_SIZE (EZM_PP_MAX_PAYLOAD_SIZE - sizeof(ezm_pp_get_block_res_t) - 2)
#define EZM_PP_MAX_MESSAGE_SIZE (EZM_PP_MAX_PAYLOAD_SIZE + sizeof(ezm_pp_tp_hdr_t) + sizeof(ezm_uart_hdr_t))

// Transport defined headers.

typedef struct __attribute__((__packed__)) {
  uint32_t magic_byte : 8;
  uint32_t pec_type : 4;
  uint32_t hdr_version : 4;
  uint32_t pkt_len : 16;
} ezm_uart_hdr_t;

// Messages

typedef union __attribute__((__packed__)) {
  ezm_pp_gen_first_msg_t gen_first_msg;
  ezm_pp_gen_next_msg_t gen_next_msg;

  ezm_pp_ctrl_msg_t ctrl_msg_req_gen;

  ezm_pp_get_version_req_t get_version_req;
  ezm_pp_get_capabilities_req_t get_capabilities_req;
  ezm_pp_set_capabilities_req_t set_capabilities_req;
  ezm_pp_get_storage_info_req_t get_storage_info_req;
  ezm_pp_get_img_info_req_t get_img_info_req;
  ezm_pp_download_req_t download_req;
  ezm_pp_get_block_req_t get_block_req;
  ezm_pp_retransmit_req_t retransmit_req;
  ezm_pp_boot_img_req_t boot_req;
  ezm_pp_remove_img_req_t remove_img_req;

  ezm_pp_ctrl_msg_res_t ctrl_msg_res_gen;

  ezm_pp_get_version_res_t get_version_res;
  ezm_pp_get_capabilities_res_t get_capabilities_res;
  ezm_pp_get_storage_info_res_t get_storage_info_res;
  ezm_pp_get_img_info_res_t get_img_info_res;
  ezm_pp_download_res_t download_res;
  ezm_pp_get_block_res_t get_block_res;

  uint8_t pp_pkt_payload[EZM_PP_MAX_PAYLOAD_SIZE];
} ezm_pp_msg_u;

// Generic packet structure.

typedef struct {
  ezm_pp_tp_hdr_t tp_hdr;
  ezm_pp_msg_u msg;
} ezm_pp_pkt_t;

// Transport specific packet structure.

typedef struct __attribute__((__packed__)) {
  ezm_uart_hdr_t tl_hdr;
  ezm_pp_pkt_t pp_pkt;
} ezm_pp_uart_pkt_t;

#endif // EZM_PP_PKT_H_