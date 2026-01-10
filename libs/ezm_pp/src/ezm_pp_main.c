#include <string.h>
#include "ezm_pp_main.h"
#include "ezm_pp_obj.h"
#include "ezm_pp_pkt_maker.h"
#include "ezm_pp_ep_drv.h"
#include "ezm_pp_pkt_processor.h"

#include "ezm_ut_utils.h"
#include "ezm_log_gen.h" 
DEFINE_TAG();

STATIC_GLOBAL uint8_t ezm_pp_tx_buffer[EZM_PP_MAX_MESSAGE_SIZE];
STATIC_GLOBAL uint8_t ezm_pp_rx_buffer[EZM_PP_MAX_MESSAGE_SIZE];


static uint32_t ezm_pp_main_send_packet(ezm_pp_obj_t *src,
                                        ezm_pp_obj_t *target, void *tx_pkt,
                                        uint32_t pkt_length) {
  if (target == NULL || src == NULL || tx_pkt == NULL || pkt_length == 0 ||
      pkt_length > EZM_PP_MAX_MESSAGE_SIZE) {
    return 0;
  }

  ezm_pp_drv_pkt_t drv_pkt = {
      .drv_pkt.pkt = tx_pkt, .drv_pkt_len = pkt_length, .drv_id = target->phy};

  uint32_t rt = ezm_pp_drv_send_pkt(&drv_pkt);
  return rt;
}

static void ezm_pp_main_req_next_block(ezm_pp_action_t action, ezm_pp_drv_msg_t *drv_msg, void *pp_tx_buffer) {
  

    uint32_t next_block = ezm_pp_ep_drv_get_next_block_number();
    
    if(action == EZM_PP_ACTION_END_DOWNLOAD) {
      next_block = 0xFFFFFFFF; // Indicate end of download
    } else if(action == EZM_PP_ACTION_INVALID_WRITE) {
      next_block = 0xDEADBEEF; // Indicate invalid write
    }

    ezm_pp_obj_t * sender = &ezm_pp_ep_obj;
    ezm_pp_obj_t * receiver = &ezm_pp_host_obj;

    uint32_t pp_pkt_length = ezm_pp_pkt_maker_get_block_req(sender, receiver, drv_msg->drv_id, pp_tx_buffer, next_block);
  
    ezm_pp_main_send_packet(sender, receiver, pp_tx_buffer, pp_pkt_length);
}

static ezm_pp_action_t ezm_pp_main_process_message(void *pp_tx_buffer, ezm_pp_drv_msg_t * drv_msg) {
  ezm_pp_action_t action = EZM_PP_ACTION_DISCARD_INVALID_PACKET;
  if (pp_tx_buffer == NULL || drv_msg == NULL) {
    return action;
  }

  memset(pp_tx_buffer, 0, sizeof(ezm_pp_tx_buffer));
  memset(ezm_pp_rx_buffer, 0, sizeof(ezm_pp_tx_buffer));

  ezm_pp_drv_pkt_t drv_pkt = {.drv_pkt.pkt = ezm_pp_rx_buffer};
  drv_pkt.drv_id = drv_msg->drv_id;

  ezm_pp_pkt_t *rx_pkt = NULL;
  ezm_pp_pkt_t *tx_pkt = NULL;
  uint32_t pp_pkt_length = 0;

  ezm_pp_obj_t *sender = NULL;
  ezm_pp_obj_t * receiver = NULL;

  drv_pkt.drv_pkt_len = ezm_pp_drv_get_pkt(&drv_pkt);
  if (drv_pkt.drv_pkt_len == 0 ||
      drv_pkt.drv_pkt_len > EZM_PP_MAX_MESSAGE_SIZE) {
    return action;
  }

  ezm_pp_pkt_processor_get_pkt(&drv_pkt, &sender, &receiver, drv_pkt.drv_pkt_len, &rx_pkt,
                               &pp_pkt_length);
  if (rx_pkt == NULL || pp_pkt_length == 0) {
    return action;
  }

  tx_pkt = ezm_pp_pkt_maker_get_pp_pkt_region(drv_pkt.drv_id, pp_tx_buffer);

  if (tx_pkt == NULL) {
    return action;
  }

  action = ezm_pp_pkt_processor_message(sender, receiver, &pp_pkt_length,
                                        rx_pkt, tx_pkt);

  if (action != EZM_PP_ACTION_DISCARD_INVALID_PACKET && pp_pkt_length > 0) {
    pp_pkt_length = ezm_pp_pkt_maker_fill_header(drv_pkt.drv_id, pp_tx_buffer,
                                                 pp_pkt_length);
    ezm_pp_main_send_packet(receiver, sender, pp_tx_buffer, pp_pkt_length);
  }

  return action;
}

ezm_pp_action_t ezm_pp_main(void) {
  ezm_pp_drv_listen();

  ezm_pp_drv_msg_t drv_msg = ezm_pp_drv_get_msg();

  // Handle event
  switch (drv_msg.ev) 
	{
    case EZM_PP_DRV_EV_NEW_MSG:
      drv_msg.action = ezm_pp_main_process_message(ezm_pp_tx_buffer, &drv_msg);
      break;
    default:
      break;
	}

  // Handle action
  switch (drv_msg.action) {
    case EZM_PP_ACTION_NO_RESPONSE:
    case EZM_PP_ACTION_DISCARD_INVALID_PACKET: {
      break;
    }
    case EZM_PP_ACTION_WAIT_FOR_NEXT_BLOCK:
    case EZM_PP_NO_ACTION: {
      break;
    }
    case EZM_PP_ACTION_IDENT_VERSION: {
      uint32_t pp_pkt_len = ezm_pp_pkt_maker_get_version_req(&ezm_pp_host_obj, &ezm_pp_ep_obj, drv_msg.drv_id, ezm_pp_tx_buffer);
      ezm_pp_main_send_packet(&ezm_pp_host_obj, &ezm_pp_ep_obj, ezm_pp_tx_buffer, pp_pkt_len);
      break;
    }
    case EZM_PP_ACTION_IDENT_CAPABILITIES: {
      uint32_t pp_pkt_len = ezm_pp_pkt_maker_get_capabilities_req(&ezm_pp_host_obj, &ezm_pp_ep_obj, drv_msg.drv_id, ezm_pp_tx_buffer);
      ezm_pp_main_send_packet(&ezm_pp_host_obj, &ezm_pp_ep_obj, ezm_pp_tx_buffer, pp_pkt_len);
      break;
    }
    case EZM_PP_ACTION_IDENT_STORAGE_INFO: {
      uint32_t pp_pkt_len = ezm_pp_pkt_maker_get_storage_info_req(&ezm_pp_host_obj, &ezm_pp_ep_obj, drv_msg.drv_id, ezm_pp_tx_buffer);
      ezm_pp_main_send_packet(&ezm_pp_host_obj, &ezm_pp_ep_obj, ezm_pp_tx_buffer, pp_pkt_len);
      break;
    }
    case EZM_PP_ACTION_IDENT_IMG_INFO: {
      uint32_t pp_pkt_len = ezm_pp_pkt_maker_get_img_info_req(&ezm_pp_host_obj, &ezm_pp_ep_obj, drv_msg.drv_id, ezm_pp_tx_buffer, (uint8_t)drv_msg.arg);
      ezm_pp_main_send_packet(&ezm_pp_host_obj, &ezm_pp_ep_obj, ezm_pp_tx_buffer, pp_pkt_len);
      break;
    }
    case EZM_PP_ACTION_PREPARE_DOWNLOAD: {
      uint32_t pp_pkt_len = ezm_pp_pkt_maker_download_req(
        &ezm_pp_host_obj, 
        &ezm_pp_ep_obj, 
        drv_msg.drv_id, 
        ezm_pp_tx_buffer,
        ezm_pp_ep_drv_get_block_size(), 
        (uint8_t)drv_msg.arg, 
        ezm_pp_ep_drv_get_img_size()
        );
      ezm_pp_main_send_packet(&ezm_pp_host_obj, &ezm_pp_ep_obj, ezm_pp_tx_buffer, pp_pkt_len);
      break;
    }
    case EZM_PP_ACTION_HOST_ERROR_DOWNLOAD:
    case EZM_PP_ACTION_HOST_END_DOWNLOAD: {
      break;
    }
    case EZM_PP_ACTION_INVALID_WRITE:
    case EZM_PP_ACTION_END_DOWNLOAD:
    case EZM_PP_ACTION_REQ_NEXT_BLOCK: {
      ezm_pp_main_req_next_block(drv_msg.action, &drv_msg, ezm_pp_tx_buffer);
      break;
    }
    case EZM_PP_ACTION_BOOT_IMG: {
      int rt = ezm_pp_ep_drv_boot_img();
      if(rt != 0) {
        print_serial_log_error("Failed to boot image. Error code: %d", rt);
      }
      break;
    }
    default:
      break;
  }
  return drv_msg.action;
}