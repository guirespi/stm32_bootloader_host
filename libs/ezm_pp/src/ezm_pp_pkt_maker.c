#include <string.h>

#include <ezm_pp_pkt_maker.h>
#include <ezm_pp_uart_drv.h>

typedef uint32_t (*ezm_pp_drv_fill_header_f)(void *buffer, uint32_t pp_pkt_len);

const ezm_pp_drv_fill_header_f tp_binding_fill_hdr[] = {
    [EZM_PP_DRV_SERIAL] = (ezm_pp_drv_fill_header_f)ezm_pp_uart_drv_fill_header};

static void ezm_pp_pkt_maker_fill_hdr(ezm_pp_obj_t * src, ezm_pp_obj_t * dst, ezm_pp_pkt_t *pkt) {
  pkt->tp_hdr.hdr_version = EZM_PP_HDR_VERSION;
  pkt->tp_hdr.reserved = 0;
  pkt->tp_hdr.src_id = src->role;
  pkt->tp_hdr.dst_id = dst->role;
  pkt->tp_hdr.seq = 0;
  pkt->tp_hdr.som = 1;
  pkt->tp_hdr.eom = 1;
}

uint32_t ezm_pp_pkt_maker_req_len(ezm_pp_pkt_t *pkt,
                                  ezm_pp_cmd_code_t cmd_code) {
  if (pkt == NULL || cmd_code > EZM_PP_CMD_RETRANSMIT) {
    return 0;
  }

  uint32_t req_len = sizeof(pkt->tp_hdr);

  switch (cmd_code) {
  case EZM_PP_CMD_GET_VERSION:
    req_len += sizeof(pkt->msg.get_version_req);
    break;
  case EZM_PP_CMD_GET_CAPABILITIES:
    req_len += sizeof(pkt->msg.get_capabilities_req);
    break;
  case EZM_PP_CMD_SET_CAPABILITIES:
    req_len += sizeof(pkt->msg.set_capabilities_req);
    break;
  case EZM_PP_CMD_GET_STORAGE_INFO:
    req_len += sizeof(pkt->msg.get_storage_info_req);
    break;
  case EZM_PP_CMD_GET_IMG_INFO:
    req_len += sizeof(pkt->msg.get_img_info_req);
    break;
  case EZM_PP_CMD_DOWNLOAD:
    req_len += sizeof(pkt->msg.download_req);
    break;
  case EZM_PP_CMD_GET_BLOCK:
    req_len += sizeof(pkt->msg.get_block_req);
    break;
  case EZM_PP_CMD_BOOT_IMG:
    req_len += sizeof(pkt->msg.boot_req);
    break;
  case EZM_PP_CMD_REMOVE_IMG:
    req_len += sizeof(pkt->msg.remove_img_req);
    break;
  case EZM_PP_CMD_RETRANSMIT:
    req_len += sizeof(pkt->msg.retransmit_req);
    break;
  default:
    req_len = 0;
    break;
  }
  return req_len;
}

uint32_t ezm_pp_pkt_maker_res_len(ezm_pp_pkt_t *pkt, ezm_pp_cmd_code_t cmd_code,
                                  ezm_pp_cmd_comp_code_t completion_code) {
  if (pkt == NULL || cmd_code > EZM_PP_CMD_RETRANSMIT) {
    return 0;
  }

  if (completion_code != EZM_PP_SUCCESS) {
    // For error responses, only the control header is returned plus response
    // code.
    return sizeof(pkt->tp_hdr) + sizeof(pkt->msg.ctrl_msg_res_gen);
  }

  uint32_t res_len = sizeof(pkt->tp_hdr);

  switch (cmd_code) {
  case EZM_PP_CMD_GET_VERSION:
    res_len += sizeof(pkt->msg.get_version_res);
    break;
  case EZM_PP_CMD_GET_CAPABILITIES:
    res_len += sizeof(pkt->msg.get_capabilities_res);
    break;
  case EZM_PP_CMD_GET_STORAGE_INFO:
    res_len += sizeof(pkt->msg.get_storage_info_res);
    break;
  case EZM_PP_CMD_GET_IMG_INFO:
    res_len += sizeof(pkt->msg.get_img_info_res);
    break;
  case EZM_PP_CMD_DOWNLOAD:
    res_len += sizeof(pkt->msg.download_res);
    break;
  case EZM_PP_CMD_GET_BLOCK:
    res_len +=
        sizeof(pkt->msg.get_block_res) + pkt->msg.get_block_res.block_size;
    break;
  case EZM_PP_CMD_SET_CAPABILITIES:
  case EZM_PP_CMD_BOOT_IMG:
  case EZM_PP_CMD_REMOVE_IMG:
  case EZM_PP_CMD_RETRANSMIT:
    res_len += sizeof(pkt->msg.ctrl_msg_res_gen);
    break;
  default:
    res_len = 0;
    break;
  }
  return res_len;
}

static void ezm_pp_pkt_maker_generic_req(ezm_pp_obj_t * src, ezm_pp_obj_t * dst, 
  ezm_pp_pkt_t *pkt, ezm_pp_cmd_code_t cmd_code) {
  ezm_pp_pkt_maker_fill_hdr(src, dst, pkt);

  pkt->msg.ctrl_msg_req_gen.cmn_hdr.msg_type = EZM_PP_MSG_TYPE_CTRL;
  pkt->msg.ctrl_msg_req_gen.cmn_hdr.ic = 0;
  pkt->msg.ctrl_msg_req_gen.rq = 1; // Request
  pkt->msg.ctrl_msg_req_gen.cmd_code = cmd_code;
}

static void
ezm_pp_pkt_maker_generic_res(ezm_pp_obj_t * src, ezm_pp_obj_t * dst, ezm_pp_pkt_t *pkt, ezm_pp_cmd_code_t cmd_code,
                             ezm_pp_cmd_comp_code_t completion_code) {
  ezm_pp_pkt_maker_fill_hdr(src, dst, pkt);

  pkt->msg.ctrl_msg_res_gen.ctrl_hdr.cmn_hdr.msg_type = EZM_PP_MSG_TYPE_CTRL;
  pkt->msg.ctrl_msg_res_gen.ctrl_hdr.cmn_hdr.ic = 0;
  pkt->msg.ctrl_msg_res_gen.ctrl_hdr.rq = 0;
  pkt->msg.ctrl_msg_res_gen.ctrl_hdr.cmd_code = cmd_code;
  pkt->msg.ctrl_msg_res_gen.cmd_res = completion_code;
}

uint32_t ezm_pp_pkt_maker_req(ezm_pp_obj_t * src, ezm_pp_obj_t * dst, ezm_pp_pkt_t *pkt, ezm_pp_cmd_code_t cmd_code) {
  if (pkt == NULL || cmd_code > EZM_PP_CMD_RETRANSMIT) {
    return 0;
  }

  uint32_t req_len = ezm_pp_pkt_maker_req_len(pkt, cmd_code);

  ezm_pp_pkt_maker_generic_req(src, dst, pkt, cmd_code);

  return req_len;
}

uint32_t ezm_pp_pkt_maker_res(ezm_pp_obj_t * src, ezm_pp_obj_t * dst, ezm_pp_pkt_t *pkt, ezm_pp_cmd_code_t cmd_code,
                              ezm_pp_cmd_comp_code_t completion_code) {
  if (pkt == NULL || cmd_code > EZM_PP_CMD_RETRANSMIT) {
    return 0;
  }

  uint32_t req_len = ezm_pp_pkt_maker_res_len(pkt, cmd_code, completion_code);

  ezm_pp_pkt_maker_generic_res(src, dst, pkt, cmd_code, completion_code);

  return req_len;
}

uint32_t ezm_pp_pkt_maker_fill_header(ezm_pp_drv_phy_tp_binding_id_t tp_binding,
                                      void *buffer, uint32_t pp_pkt_len) {
  if (tp_binding > EZM_PP_DRV_SERIAL || buffer == NULL) {
    return 0;
  }

  if (tp_binding_fill_hdr[tp_binding] == NULL) {
    return 0;
  }

  return tp_binding_fill_hdr[tp_binding](buffer, pp_pkt_len);
}

ezm_pp_pkt_t *
ezm_pp_pkt_maker_get_pp_pkt_region(ezm_pp_drv_phy_tp_binding_id_t tp_binding,
                                   void *buffer) {
  ezm_pp_pkt_t *pp_pkt = NULL;

  if (buffer == NULL) {
    return NULL;
  }

  if (tp_binding == EZM_PP_DRV_SERIAL) {
    pp_pkt = &(((ezm_pp_uart_pkt_t *)(uintptr_t)buffer)->pp_pkt);
  }

  return pp_pkt;
}

uint32_t ezm_pp_pkt_maker_get_version_req(ezm_pp_obj_t * src, ezm_pp_obj_t * dst, ezm_pp_drv_phy_tp_binding_id_t tp_binding , void * buffer) {
  if (buffer == NULL) {
    return 0;
  }

  uint32_t pp_pkt_len = 0;
  ezm_pp_pkt_t *pkt =
      ezm_pp_pkt_maker_get_pp_pkt_region(tp_binding, buffer);
  if (pkt == NULL) {
    return 0;
  }

  pp_pkt_len = ezm_pp_pkt_maker_req(src, dst, pkt, EZM_PP_CMD_GET_VERSION);
  pp_pkt_len = ezm_pp_pkt_maker_fill_header(tp_binding, buffer, pp_pkt_len);

  return pp_pkt_len;
}

uint32_t ezm_pp_pkt_maker_get_storage_info_req(ezm_pp_obj_t * src, ezm_pp_obj_t * dst, ezm_pp_drv_phy_tp_binding_id_t tp_binding , void * buffer) {
  if (buffer == NULL) {
    return 0;
  }

  uint32_t pp_pkt_len = 0;
  ezm_pp_pkt_t *pkt =
      ezm_pp_pkt_maker_get_pp_pkt_region(tp_binding, buffer);
  if (pkt == NULL) {
    return 0;
  }

  pp_pkt_len = ezm_pp_pkt_maker_req(src, dst, pkt, EZM_PP_CMD_GET_STORAGE_INFO);
  pp_pkt_len = ezm_pp_pkt_maker_fill_header(tp_binding, buffer, pp_pkt_len);

  return pp_pkt_len;
}

uint32_t ezm_pp_pkt_maker_get_capabilities_req(ezm_pp_obj_t * src, ezm_pp_obj_t * dst, ezm_pp_drv_phy_tp_binding_id_t tp_binding , void * buffer) {
  if (buffer == NULL) {
    return 0;
  }

  uint32_t pp_pkt_len = 0;
  ezm_pp_pkt_t *pkt =
      ezm_pp_pkt_maker_get_pp_pkt_region(tp_binding, buffer);
  if (pkt == NULL) {
    return 0;
  }

  pp_pkt_len = ezm_pp_pkt_maker_req(src, dst, pkt, EZM_PP_CMD_GET_CAPABILITIES);
  pp_pkt_len = ezm_pp_pkt_maker_fill_header(tp_binding, buffer, pp_pkt_len);

  return pp_pkt_len;
}

uint32_t ezm_pp_pkt_maker_set_capabilities_req(ezm_pp_obj_t * src, ezm_pp_obj_t * dst, ezm_pp_drv_phy_tp_binding_id_t tp_binding , void * buffer, uint32_t max_data_transfer, uint32_t max_message_size) {
  if (buffer == NULL) {
    return 0;
  }

  uint32_t pp_pkt_len = 0;
  ezm_pp_pkt_t *pkt =
      ezm_pp_pkt_maker_get_pp_pkt_region(tp_binding, buffer);
  if (pkt == NULL) {
    return 0;
  }

  pp_pkt_len = ezm_pp_pkt_maker_req(src, dst, pkt, EZM_PP_CMD_SET_CAPABILITIES);

  pkt->msg.set_capabilities_req.max_data_transfer = max_data_transfer;
  pkt->msg.set_capabilities_req.max_message_size = max_message_size;

  pp_pkt_len = ezm_pp_pkt_maker_fill_header(tp_binding, buffer, pp_pkt_len);

  return pp_pkt_len;
}

uint32_t ezm_pp_pkt_maker_get_img_info_req(ezm_pp_obj_t * src, ezm_pp_obj_t * dst, ezm_pp_drv_phy_tp_binding_id_t tp_binding , void * buffer, uint8_t img_index) {
  if (buffer == NULL) {
    return 0;
  }

  uint32_t pp_pkt_len = 0;
  ezm_pp_pkt_t *pkt =
      ezm_pp_pkt_maker_get_pp_pkt_region(tp_binding, buffer);
  if (pkt == NULL) {
    return 0;
  }

  pp_pkt_len = ezm_pp_pkt_maker_req(src, dst, pkt, EZM_PP_CMD_GET_IMG_INFO);

  pkt->msg.get_img_info_req.img_index = img_index;

  pp_pkt_len = ezm_pp_pkt_maker_fill_header(tp_binding, buffer, pp_pkt_len);

  return pp_pkt_len;
}

uint32_t ezm_pp_pkt_maker_download_req(ezm_pp_obj_t * src, ezm_pp_obj_t * dst, ezm_pp_drv_phy_tp_binding_id_t tp_binding , void * buffer, uint32_t block_size, uint8_t img_idx, uint32_t img_size) {
  if (buffer == NULL) {
    return 0;
  }

  uint32_t pp_pkt_len = 0;
  ezm_pp_pkt_t *pkt =
      ezm_pp_pkt_maker_get_pp_pkt_region(tp_binding, buffer);
  if (pkt == NULL) {
    return 0;
  }

  pp_pkt_len = ezm_pp_pkt_maker_req(src, dst, pkt, EZM_PP_CMD_DOWNLOAD);

  pkt->msg.download_req.block_size = block_size;
  pkt->msg.download_req.img_idx = img_idx;
  pkt->msg.download_req.img_size = img_size;

  pp_pkt_len = ezm_pp_pkt_maker_fill_header(tp_binding, buffer, pp_pkt_len);

  return pp_pkt_len;
}

uint32_t ezm_pp_pkt_maker_boot_img_req(ezm_pp_obj_t * src, ezm_pp_obj_t * dst, ezm_pp_drv_phy_tp_binding_id_t tp_binding , void * buffer, uint8_t img_idx) {
  if (buffer == NULL) {
    return 0;
  }

  uint32_t pp_pkt_len = 0;
  ezm_pp_pkt_t *pkt =
      ezm_pp_pkt_maker_get_pp_pkt_region(tp_binding, buffer);
  if (pkt == NULL) {
    return 0;
  }

  pp_pkt_len = ezm_pp_pkt_maker_req(src, dst, pkt, EZM_PP_CMD_BOOT_IMG);

  pkt->msg.boot_req.img_index = img_idx;

  pp_pkt_len = ezm_pp_pkt_maker_fill_header(tp_binding, buffer, pp_pkt_len);

  return pp_pkt_len;
}

uint32_t ezm_pp_pkt_maker_get_block_rsp(ezm_pp_obj_t * src, ezm_pp_obj_t * dst, ezm_pp_drv_phy_tp_binding_id_t tp_binding , void * buffer, ezm_pp_cmd_comp_code_t completion_code, uint32_t block_number, uint32_t block_size, uint8_t * block_data) {
  if (buffer == NULL) {
    return 0;
  }

  if(completion_code != EZM_PP_SUCCESS && (block_size == 0 || block_size > EZM_PP_MAX_BLOCK_SIZE || block_data == NULL)) {
    return 0;
  }

  uint32_t pp_pkt_len = 0;
  ezm_pp_pkt_t *pkt =
      ezm_pp_pkt_maker_get_pp_pkt_region(tp_binding, buffer);
  if (pkt == NULL) {
    return 0;
  }

  pkt->msg.get_block_res.block_size = 0;
  pkt->msg.get_block_res.block_number = 0;

  if(completion_code == EZM_PP_SUCCESS) {
    memcpy((void *)pkt->msg.get_block_res.block_data, (void *)block_data, block_size);
    pkt->msg.get_block_res.block_size = block_size;
    pkt->msg.get_block_res.block_number = block_number;
  }
  
  pp_pkt_len = ezm_pp_pkt_maker_res(src, dst, pkt, EZM_PP_CMD_GET_BLOCK, completion_code);

  pp_pkt_len = ezm_pp_pkt_maker_fill_header(tp_binding, buffer, pp_pkt_len);

  return pp_pkt_len;
}

uint32_t ezm_pp_pkt_maker_get_block_req(ezm_pp_obj_t * src, ezm_pp_obj_t * dst, ezm_pp_drv_phy_tp_binding_id_t tp_binding , void * buffer, uint32_t block_number) {
  if (buffer == NULL) {
    return 0;
  }

  uint32_t pp_pkt_len = 0;
  ezm_pp_pkt_t *pkt =
      ezm_pp_pkt_maker_get_pp_pkt_region(tp_binding, buffer);
  if (pkt == NULL) {
    return 0;
  }

  pkt->msg.get_block_req.block_number = block_number;
  
  pp_pkt_len = ezm_pp_pkt_maker_req(src, dst, pkt, EZM_PP_CMD_GET_BLOCK);

  pp_pkt_len = ezm_pp_pkt_maker_fill_header(tp_binding, buffer, pp_pkt_len);

  return pp_pkt_len;
}
