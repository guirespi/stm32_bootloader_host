#ifndef EZM_PP_H_
#define EZM_PP_H_

#include <stdint.h>

#define EZM_PP_HDR_VERSION (0x00)
#define EZM_PP_IMG_NAME_SIZE 16
#define EZM_PP_IMG_VERSION_SIZE 4

typedef enum {
  EZM_PP_NO_ACTION = 0,
  EZM_PP_ACTION_NO_RESPONSE,
  EZM_PP_ACTION_IDENT_VERSION,
  EZM_PP_ACTION_IDENT_CAPABILITIES,
  EZM_PP_ACTION_IDENT_STORAGE_INFO,
  EZM_PP_ACTION_IDENT_IMG_INFO,
  EZM_PP_ACTION_DISCARD_INVALID_PACKET,
  EZM_PP_ACTION_RETRANSMIT,
  EZM_PP_ACTION_CHANGE_CAPABILITIES,
  EZM_PP_ACTION_PREPARE_DOWNLOAD,
  EZM_PP_ACTION_WAIT_FOR_NEXT_BLOCK,
  EZM_PP_ACTION_REQ_NEXT_BLOCK,
  EZM_PP_ACTION_WRITE_BLOCK,
  EZM_PP_ACTION_READ_BLOCK,
  EZM_PP_ACTION_READ_IMG_HEADER,
  EZM_PP_ACTION_BOOT_IMG,
  EZM_PP_ACTION_REMOVE_IMG,
  EZM_PP_ACTION_INVALID_WRITE,
  EZM_PP_ACTION_END_DOWNLOAD,
  EZM_PP_ACTION_HOST_END_DOWNLOAD,
  EZM_PP_ACTION_HOST_ERROR_DOWNLOAD,
} ezm_pp_action_t;

typedef enum __attribute__((__packed__)) {
  EZM_PP_SUCCESS = 0x00,
  EZM_PP_ERROR = 0x01,
  EZM_PP_ERROR_INVALID_DATA = 0x02,
  EZM_PP_ERROR_INVALID_LENGTH = 0x03,
  EZM_PP_ERROR_NOT_READ = 0x04,
  EZM_PP_ERROR_UNSUPPORTED_CMD = 0x05,
  EZM_PP_ERROR_NO_RESPONSE = 0xff,
} ezm_pp_cmd_comp_code_t;

typedef enum __attribute__((__packed__)) {
  EZM_PP_CMD_GET_VERSION = 0x00,
  EZM_PP_CMD_GET_CAPABILITIES = 0x01,
  EZM_PP_CMD_SET_CAPABILITIES = 0x02,
  EZM_PP_CMD_GET_STORAGE_INFO = 0x03,
  EZM_PP_CMD_GET_IMG_INFO = 0x04,
  EZM_PP_CMD_DOWNLOAD = 0x05,
  EZM_PP_CMD_GET_BLOCK = 0x06,
  EZM_PP_CMD_BOOT_IMG = 0x07,
  EZM_PP_CMD_REMOVE_IMG = 0x08,
  EZM_PP_CMD_RETRANSMIT = 0x0f,
  EZM_PP_CMD_MAX,
} ezm_pp_cmd_code_t;

typedef enum __attribute__((__packed__)) {
  EZM_PP_MSG_TYPE_CTRL = 0x00,
} ezm_msg_type;

typedef struct __attribute__((__packed__)) {
  uint8_t major;
  uint8_t minor;
  uint8_t patch;
} ezm_pp_cmd_version_t;

typedef struct __attribute__((__packed__)) {
  uint32_t hdr_version : 4;
  uint32_t reserved : 4;
  uint32_t src_id : 8;
  uint32_t dst_id : 8;
  uint32_t : 4;
  uint32_t seq : 2;
  uint32_t eom : 1;
  uint32_t som : 1;
} ezm_pp_tp_hdr_t;

typedef struct __attribute__((__packed__)) {
  uint8_t msg_type : 7;
  uint8_t ic : 1;
} ezm_pp_msg_common_t;

typedef struct __attribute__((__packed__)) {
  ezm_pp_msg_common_t cmn_hdr;
  uint8_t : 7;
  uint8_t rq : 1;
  uint8_t cmd_code;
} ezm_pp_ctrl_msg_t;

typedef struct __attribute__((__packed__)) {
  ezm_pp_ctrl_msg_t ctrl_hdr;
  uint8_t cmd_res;
} ezm_pp_ctrl_msg_res_t;

typedef struct __attribute__((__packed__)) {
  ezm_pp_msg_common_t cmn_hdr;
  uint8_t payload[];
} ezm_pp_gen_first_msg_t;

typedef struct __attribute__((__packed__)) {
  uint8_t payload_first_byte;
  uint8_t payload[];
} ezm_pp_gen_next_msg_t;

typedef struct __attribute__((__packed__)) {
  ezm_pp_ctrl_msg_t ctrl_hdr;
  uint8_t reserved;
} ezm_pp_get_version_req_t;

typedef struct __attribute__((__packed__)) {
  ezm_pp_ctrl_msg_res_t ctrl_hdr;
  ezm_pp_cmd_version_t version;
} ezm_pp_get_version_res_t;

typedef struct __attribute__((__packed__)) {
  ezm_pp_ctrl_msg_t ctrl_hdr;
  uint8_t reserved;
} ezm_pp_get_capabilities_req_t;

typedef struct __attribute__((__packed__)) {
  ezm_pp_ctrl_msg_res_t ctrl_hdr;
  uint32_t max_data_transfer;
  uint32_t max_message_size;
} ezm_pp_get_capabilities_res_t;

typedef struct __attribute__((__packed__)) {
  ezm_pp_ctrl_msg_t ctrl_hdr;
  uint32_t max_data_transfer;
  uint32_t max_message_size;
} ezm_pp_set_capabilities_req_t;

typedef struct __attribute__((__packed__)) {
  ezm_pp_ctrl_msg_t ctrl_hdr;
  uint8_t reserved;
} ezm_pp_get_storage_info_req_t;

typedef struct __attribute__((__packed__)) {
  ezm_pp_ctrl_msg_res_t ctrl_hdr;
  uint32_t storage_size;
  uint32_t img_mask;
} ezm_pp_get_storage_info_res_t;

typedef struct __attribute__((__packed__)) {
  ezm_pp_ctrl_msg_t ctrl_hdr;
  uint8_t img_index;
} ezm_pp_get_img_info_req_t;

typedef struct __attribute__((__packed__)) {
  ezm_pp_ctrl_msg_res_t ctrl_hdr;
  uint16_t  img_status;
  uint16_t  img_type;
  uint8_t   img_name[EZM_PP_IMG_NAME_SIZE];
  uint32_t  img_size;
  uint8_t   img_version[EZM_PP_IMG_VERSION_SIZE];
} ezm_pp_get_img_info_res_t;

typedef struct __attribute__((__packed__)) {
  ezm_pp_ctrl_msg_t ctrl_hdr;
  uint8_t img_idx;
  uint32_t img_size;
  uint32_t block_size;
} ezm_pp_download_req_t;

typedef struct __attribute__((__packed__)) {
  ezm_pp_ctrl_msg_res_t ctrl_hdr;
  uint32_t block_size;
  uint32_t block_number;
} ezm_pp_download_res_t;

typedef struct __attribute__((__packed__)) {
  ezm_pp_ctrl_msg_t ctrl_hdr;
  uint32_t block_number;
} ezm_pp_get_block_req_t;

typedef struct __attribute__((__packed__)) {
  ezm_pp_ctrl_msg_res_t ctrl_hdr;
  uint32_t block_number;
  uint32_t block_size;
  uint8_t block_data[];
} ezm_pp_get_block_res_t;

typedef struct __attribute__((__packed__)){
  ezm_pp_ctrl_msg_t ctrl_hdr;
  uint8_t reserved;
} ezm_pp_retransmit_req_t;

typedef struct __attribute__((__packed__)) {
  ezm_pp_ctrl_msg_t ctrl_hdr;
  uint8_t img_index;
} ezm_pp_boot_img_req_t;

typedef struct __attribute__((__packed__)){
  ezm_pp_ctrl_msg_t ctrl_hdr;
  uint8_t img_index;
} ezm_pp_remove_img_req_t;

#endif // EZM_PP_H_