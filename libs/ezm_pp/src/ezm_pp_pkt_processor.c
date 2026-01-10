#include "ezm_pp_pkt_maker.h"
#include "ezm_pp_pkt_processor.h"
#include "ezm_pp_uart_drv.h"
#include "ezm_pp_ep_drv.h"
#include <ezm_log_gen.h>
DEFINE_TAG();

#define EZM_PP_PKT_PROCESSOR_CHECK_PROCESS_CMD_ARG(receiver, sender, rx_pkt,   \
                                                   tx_pkt, comp_code)          \
  do {                                                                         \
    if (rx_pkt == NULL || tx_pkt == NULL || comp_code == NULL ||               \
        receiver == NULL || sender == NULL) {                                  \
      return EZM_PP_ACTION_NO_RESPONSE;                                        \
    }                                                                          \
  } while (0)

typedef void (*ezm_pp_drv_extract_pp_pkt_f)(void *buffer, uint32_t tlp_length,
                                            ezm_pp_obj_t ** sender, ezm_pp_obj_t ** receiver, 
                                            ezm_pp_pkt_t **pp_pkt,
                                            uint32_t *pp_pkt_length);

/**
 * @brief Extracts a pp packet from a provided buffer.
 *
 * @param buffer Pointer to the input buffer containing the packet data.
 * @param tlp_length Length of the transport layer packet.
 * @param pp_pkt Pointer to the output pointer for the parsed packet structure.
 * @param pp_pkt_length Pointer to the output length of the parsed packet.
 */
static void pp_pkt_processor_get_pp_pkt_serial(void *buffer,
                                               uint32_t tlp_length,
                                               ezm_pp_obj_t **sender, 
                                               ezm_pp_obj_t **receiver,
                                               ezm_pp_pkt_t **pp_pkt,
                                               uint32_t *pp_pkt_length);

typedef ezm_pp_action_t (*ezm_pp_process_cmd_f)(
    ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver, ezm_pp_pkt_t *rx_pkt,
    ezm_pp_pkt_t *tx_pkt, ezm_pp_cmd_comp_code_t *comp_code);

/**
 * @brief Processes a packet to retrieve the protocol version information.
 *
 * @param receiver Pointer to the receiver driver object.
 * @param sender Pointer to the sender driver object.
 * @param rx_pkt Pointer to the received packet.
 * @param tx_pkt Pointer to the packet to be transmitted in response.
 * @param comp_code Pointer to the completion code for the command.
 * @return Action to be taken after processing.
 */
static ezm_pp_action_t pp_pkt_processor_get_version(
    ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver, ezm_pp_pkt_t *rx_pkt,
    ezm_pp_pkt_t *tx_pkt, ezm_pp_cmd_comp_code_t *comp_code);

/**
 * @brief Processes a packet to retrieve device capabilities.
 *
 * @param receiver Pointer to the receiver driver object.
 * @param sender Pointer to the sender driver object.
 * @param rx_pkt Pointer to the received packet.
 * @param tx_pkt Pointer to the packet to be transmitted in response.
 * @param comp_code Pointer to the completion code for the command.
 * @return Action to be taken after processing.
 */
static ezm_pp_action_t pp_pkt_processor_get_capabilities(
    ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver, ezm_pp_pkt_t *rx_pkt,
    ezm_pp_pkt_t *tx_pkt, ezm_pp_cmd_comp_code_t *comp_code);

/**
 * @brief Processes a packet to set device capabilities.
 *
 * @param receiver Pointer to the receiver driver object.
 * @param sender Pointer to the sender driver object.
 * @param rx_pkt Pointer to the received packet.
 * @param tx_pkt Pointer to the packet to be transmitted in response.
 * @param comp_code Pointer to the completion code for the command.
 * @return Action to be taken after processing.
 */
static ezm_pp_action_t pp_pkt_processor_set_capabilities(
    ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver, ezm_pp_pkt_t *rx_pkt,
    ezm_pp_pkt_t *tx_pkt, ezm_pp_cmd_comp_code_t *comp_code);

/**
 * @brief Processes a packet to retrieve storage information.
 *
 * @param receiver Pointer to the receiver driver object.
 * @param sender Pointer to the sender driver object.
 * @param rx_pkt Pointer to the received packet.
 * @param tx_pkt Pointer to the packet to be transmitted in response.
 * @param comp_code Pointer to the completion code for the command.
 * @return Action to be taken after processing.
 */
static ezm_pp_action_t pp_pkt_processor_get_storage_info(
    ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver, ezm_pp_pkt_t *rx_pkt,
    ezm_pp_pkt_t *tx_pkt, ezm_pp_cmd_comp_code_t *comp_code);

/**
 * @brief Processes a packet to retrieve image information.
 *
 * @param receiver Pointer to the receiver driver object.
 * @param sender Pointer to the sender driver object.
 * @param rx_pkt Pointer to the received packet.
 * @param tx_pkt Pointer to the packet to be transmitted in response.
 * @param comp_code Pointer to the completion code for the command.
 * @return Action to be taken after processing.
 */
static ezm_pp_action_t pp_pkt_processor_get_img_info(
    ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver, ezm_pp_pkt_t *rx_pkt,
    ezm_pp_pkt_t *tx_pkt, ezm_pp_cmd_comp_code_t *comp_code);

/**
 * @brief Processes a packet to initiate image download.
 *
 * @param receiver Pointer to the receiver driver object.
 * @param sender Pointer to the sender driver object.
 * @param rx_pkt Pointer to the received packet.
 * @param tx_pkt Pointer to the packet to be transmitted in response.
 * @param comp_code Pointer to the completion code for the command.
 * @return Action to be taken after processing.
 */
static ezm_pp_action_t
pp_pkt_processor_download(ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver,
                          ezm_pp_pkt_t *rx_pkt, ezm_pp_pkt_t *tx_pkt,
                          ezm_pp_cmd_comp_code_t *comp_code);

/**
 * @brief Processes a packet to retrieve a data block.
 *
 * @param receiver Pointer to the receiver driver object.
 * @param sender Pointer to the sender driver object.
 * @param rx_pkt Pointer to the received packet.
 * @param tx_pkt Pointer to the packet to be transmitted in response.
 * @param comp_code Pointer to the completion code for the command.
 * @return Action to be taken after processing.
 */
static ezm_pp_action_t
pp_pkt_processor_get_block(ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver,
                           ezm_pp_pkt_t *rx_pkt, ezm_pp_pkt_t *tx_pkt,
                           ezm_pp_cmd_comp_code_t *comp_code);

/**
 * @brief Processes a packet to boot an image.
 *
 * @param receiver Pointer to the receiver driver object.
 * @param sender Pointer to the sender driver object.
 * @param rx_pkt Pointer to the received packet.
 * @param tx_pkt Pointer to the packet to be transmitted in response.
 * @param comp_code Pointer to the completion code for the command.
 * @return Action to be taken after processing.
 */
static ezm_pp_action_t
pp_pkt_processor_boot_img(ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver,
                          ezm_pp_pkt_t *rx_pkt, ezm_pp_pkt_t *tx_pkt,
                          ezm_pp_cmd_comp_code_t *comp_code);

/**
 * @brief Processes a packet to remove an image.
 *
 * @param receiver Pointer to the receiver driver object.
 * @param sender Pointer to the sender driver object.
 * @param rx_pkt Pointer to the received packet.
 * @param tx_pkt Pointer to the packet to be transmitted in response.
 * @param comp_code Pointer to the completion code for the command.
 * @return Action to be taken after processing.
 */
static ezm_pp_action_t pp_pkt_processor_remove_img(
    ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver, ezm_pp_pkt_t *rx_pkt,
    ezm_pp_pkt_t *tx_pkt, ezm_pp_cmd_comp_code_t *comp_code);

/**
 * @brief Processes a packet to retransmit data.
 *
 * @param receiver Pointer to the receiver driver object.
 * @param sender Pointer to the sender driver object.
 * @param rx_pkt Pointer to the received packet.
 * @param tx_pkt Pointer to the packet to be transmitted in response.
 * @param comp_code Pointer to the completion code for the command.
 * @return Action to be taken after processing.
 */
static ezm_pp_action_t pp_pkt_processor_retransmit(
    ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver, ezm_pp_pkt_t *rx_pkt,
    ezm_pp_pkt_t *tx_pkt, ezm_pp_cmd_comp_code_t *comp_code);

/**
 * @brief Processes a response packet to retrieve the protocol version
 * information.
 *
 * @param receiver Pointer to the receiver driver object.
 * @param sender Pointer to the sender driver object.
 * @param rx_pkt Pointer to the received packet.
 * @param tx_pkt Pointer to the packet to be transmitted in response.
 * @param comp_code Pointer to the completion code for the command.
 * @return Action to be taken after processing.
 */
static ezm_pp_action_t pp_pkt_processor_get_version_res(
    ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver, ezm_pp_pkt_t *rx_pkt,
    ezm_pp_pkt_t *tx_pkt, ezm_pp_cmd_comp_code_t *comp_code);

/**
 * @brief Processes a response packet to retrieve device capabilities.
 *
 * @param receiver Pointer to the receiver driver object.
 * @param sender Pointer to the sender driver object.
 * @param rx_pkt Pointer to the received packet.
 * @param tx_pkt Pointer to the packet to be transmitted in response.
 * @param comp_code Pointer to the completion code for the command.
 * @return Action to be taken after processing.
 */
static ezm_pp_action_t pp_pkt_processor_get_capabilities_res(
    ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver, ezm_pp_pkt_t *rx_pkt,
    ezm_pp_pkt_t *tx_pkt, ezm_pp_cmd_comp_code_t *comp_code);

/**
 * @brief Processes a response packet to set device capabilities.
 *
 * @param receiver Pointer to the receiver driver object.
 * @param sender Pointer to the sender driver object.
 * @param rx_pkt Pointer to the received packet.
 * @param tx_pkt Pointer to the packet to be transmitted in response.
 * @param comp_code Pointer to the completion code for the command.
 * @return Action to be taken after processing.
 */
static ezm_pp_action_t pp_pkt_processor_set_capabilities_res(
    ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver, ezm_pp_pkt_t *rx_pkt,
    ezm_pp_pkt_t *tx_pkt, ezm_pp_cmd_comp_code_t *comp_code);

/**
 * @brief Processes a response packet to retrieve storage information.
 *
 * @param receiver Pointer to the receiver driver object.
 * @param sender Pointer to the sender driver object.
 * @param rx_pkt Pointer to the received packet.
 * @param tx_pkt Pointer to the packet to be transmitted in response.
 * @param comp_code Pointer to the completion code for the command.
 * @return Action to be taken after processing.
 */
static ezm_pp_action_t pp_pkt_processor_get_storage_info_res(
    ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver, ezm_pp_pkt_t *rx_pkt,
    ezm_pp_pkt_t *tx_pkt, ezm_pp_cmd_comp_code_t *comp_code);

/**
 * @brief Processes a response packet to retrieve image information.
 *
 * @param receiver Pointer to the receiver driver object.
 * @param sender Pointer to the sender driver object.
 * @param rx_pkt Pointer to the received packet.
 * @param tx_pkt Pointer to the packet to be transmitted in response.
 * @param comp_code Pointer to the completion code for the command.
 * @return Action to be taken after processing.
 */
static ezm_pp_action_t pp_pkt_processor_get_img_info_res(
    ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver, ezm_pp_pkt_t *rx_pkt,
    ezm_pp_pkt_t *tx_pkt, ezm_pp_cmd_comp_code_t *comp_code);

/**
 * @brief Processes a response packet to initiate image download.
 *
 * @param receiver Pointer to the receiver driver object.
 * @param sender Pointer to the sender driver object.
 * @param rx_pkt Pointer to the received packet.
 * @param tx_pkt Pointer to the packet to be transmitted in response.
 * @param comp_code Pointer to the completion code for the command.
 * @return Action to be taken after processing.
 */
static ezm_pp_action_t pp_pkt_processor_download_res(
    ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver, ezm_pp_pkt_t *rx_pkt,
    ezm_pp_pkt_t *tx_pkt, ezm_pp_cmd_comp_code_t *comp_code);

/**
 * @brief Processes a response packet to retrieve a data block.
 *
 * @param receiver Pointer to the receiver driver object.
 * @param sender Pointer to the sender driver object.
 * @param rx_pkt Pointer to the received packet.
 * @param tx_pkt Pointer to the packet to be transmitted in response.
 * @param comp_code Pointer to the completion code for the command.
 * @return Action to be taken after processing.
 */
static ezm_pp_action_t pp_pkt_processor_get_block_res(
    ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver, ezm_pp_pkt_t *rx_pkt,
    ezm_pp_pkt_t *tx_pkt, ezm_pp_cmd_comp_code_t *comp_code);

/**
 * @brief Processes a response packet to boot an image.
 *
 * @param receiver Pointer to the receiver driver object.
 * @param sender Pointer to the sender driver object.
 * @param rx_pkt Pointer to the received packet.
 * @param tx_pkt Pointer to the packet to be transmitted in response.
 * @param comp_code Pointer to the completion code for the command.
 * @return Action to be taken after processing.
 */
static ezm_pp_action_t pp_pkt_processor_boot_img_res(
    ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver, ezm_pp_pkt_t *rx_pkt,
    ezm_pp_pkt_t *tx_pkt, ezm_pp_cmd_comp_code_t *comp_code);

/**
 * @brief Processes a response packet to remove an image.
 *
 * @param receiver Pointer to the receiver driver object.
 * @param sender Pointer to the sender driver object.
 * @param rx_pkt Pointer to the received packet.
 * @param tx_pkt Pointer to the packet to be transmitted in response.
 * @param comp_code Pointer to the completion code for the command.
 * @return Action to be taken after processing.
 */
static ezm_pp_action_t pp_pkt_processor_remove_img_res(
    ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver, ezm_pp_pkt_t *rx_pkt,
    ezm_pp_pkt_t *tx_pkt, ezm_pp_cmd_comp_code_t *comp_code);

const ezm_pp_cmd_version_t EZM_PP_VERSION = {
    .major = 1,
    .minor = 0,
    .patch = 0,
};

const ezm_pp_drv_extract_pp_pkt_f tp_binding_extract_pp_pkt[] = {
    [EZM_PP_DRV_SERIAL] = pp_pkt_processor_get_pp_pkt_serial};

const ezm_pp_process_cmd_f pp_process_cmd[] = {
    [EZM_PP_CMD_GET_VERSION] = pp_pkt_processor_get_version,
    [EZM_PP_CMD_GET_CAPABILITIES] = pp_pkt_processor_get_capabilities,
    [EZM_PP_CMD_SET_CAPABILITIES] = pp_pkt_processor_set_capabilities,
    [EZM_PP_CMD_GET_STORAGE_INFO] = pp_pkt_processor_get_storage_info,
    [EZM_PP_CMD_GET_IMG_INFO] = pp_pkt_processor_get_img_info,
    [EZM_PP_CMD_DOWNLOAD] = pp_pkt_processor_download,
    [EZM_PP_CMD_GET_BLOCK] = pp_pkt_processor_get_block,
    [EZM_PP_CMD_BOOT_IMG] = pp_pkt_processor_boot_img,
    [EZM_PP_CMD_REMOVE_IMG] = pp_pkt_processor_remove_img,
    [EZM_PP_CMD_RETRANSMIT] = pp_pkt_processor_retransmit,
};

const ezm_pp_process_cmd_f pp_process_cmd_res[] = {
    [EZM_PP_CMD_GET_VERSION] = pp_pkt_processor_get_version_res,
    [EZM_PP_CMD_GET_CAPABILITIES] = pp_pkt_processor_get_capabilities_res,
    [EZM_PP_CMD_SET_CAPABILITIES] = pp_pkt_processor_set_capabilities_res,
    [EZM_PP_CMD_GET_STORAGE_INFO] = pp_pkt_processor_get_storage_info_res,
    [EZM_PP_CMD_GET_IMG_INFO] = pp_pkt_processor_get_img_info_res,
    [EZM_PP_CMD_DOWNLOAD] = pp_pkt_processor_download_res,
    [EZM_PP_CMD_GET_BLOCK] = pp_pkt_processor_get_block_res,
    [EZM_PP_CMD_BOOT_IMG] = pp_pkt_processor_boot_img_res,
    [EZM_PP_CMD_REMOVE_IMG] = pp_pkt_processor_remove_img_res,
};

static void pp_pkt_processor_get_pp_pkt_serial(void *buffer,
                                               uint32_t tlp_length,
                                               ezm_pp_obj_t ** sender, 
                                               ezm_pp_obj_t ** receiver, 
                                               ezm_pp_pkt_t **pp_pkt,
                                               uint32_t *pp_pkt_length) {
  if (tlp_length == 0 || pp_pkt == NULL || pp_pkt_length == NULL) {
    return;
  }

  ezm_pp_uart_pkt_t *uart_pkt = (ezm_pp_uart_pkt_t *)buffer;

  *sender = ezm_pp_obj_get_serial_sender(uart_pkt);
  *receiver = ezm_pp_obj_get_serial_receiver(uart_pkt);

  if( *sender == NULL || *receiver == NULL) {
    return;
  }

  *pp_pkt = &uart_pkt->pp_pkt;
  *pp_pkt_length = tlp_length - sizeof(uart_pkt->tl_hdr);
}

void ezm_pp_pkt_processor_get_pkt(const ezm_pp_drv_pkt_t *drv_pkt,
                                  ezm_pp_obj_t ** sender, ezm_pp_obj_t ** receiver, 
                                  uint32_t tlp_length, ezm_pp_pkt_t **pp_pkt,
                                  uint32_t *pp_pkt_length) {
  *pp_pkt_length = 0;
  *pp_pkt = NULL;
  if (drv_pkt == NULL || pp_pkt == NULL) {
    return;
  }

  if (drv_pkt->drv_id > EZM_PP_DRV_SERIAL ||
      tp_binding_extract_pp_pkt[drv_pkt->drv_id] == NULL) {
    return;
  }

  tp_binding_extract_pp_pkt[drv_pkt->drv_id](drv_pkt->drv_pkt.pkt, tlp_length, sender, receiver,
                                             pp_pkt, pp_pkt_length);
}

static ezm_pp_action_t pp_pkt_processor_get_version(
    ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver, ezm_pp_pkt_t *rx_pkt,
    ezm_pp_pkt_t *tx_pkt, ezm_pp_cmd_comp_code_t *comp_code) {
  EZM_PP_PKT_PROCESSOR_CHECK_PROCESS_CMD_ARG(receiver, sender, rx_pkt, tx_pkt,
                                             comp_code);

  ezm_pp_action_t action = EZM_PP_NO_ACTION;
  *comp_code = EZM_PP_SUCCESS;

  tx_pkt->msg.get_version_res.version = EZM_PP_VERSION;

  return action;
}

static ezm_pp_action_t pp_pkt_processor_get_capabilities(
    ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver, ezm_pp_pkt_t *rx_pkt,
    ezm_pp_pkt_t *tx_pkt, ezm_pp_cmd_comp_code_t *comp_code) {
  EZM_PP_PKT_PROCESSOR_CHECK_PROCESS_CMD_ARG(receiver, sender, rx_pkt, tx_pkt,
                                             comp_code);

  ezm_pp_action_t action = EZM_PP_NO_ACTION;
  *comp_code = EZM_PP_SUCCESS;

  tx_pkt->msg.get_capabilities_res.max_data_transfer = EZM_PP_MAX_PAYLOAD_SIZE;
  tx_pkt->msg.get_capabilities_res.max_message_size = EZM_PP_MAX_PAYLOAD_SIZE;

  return action;
}

static ezm_pp_action_t pp_pkt_processor_set_capabilities(
    ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver, ezm_pp_pkt_t *rx_pkt,
    ezm_pp_pkt_t *tx_pkt, ezm_pp_cmd_comp_code_t *comp_code) {
  EZM_PP_PKT_PROCESSOR_CHECK_PROCESS_CMD_ARG(receiver, sender, rx_pkt, tx_pkt,
                                             comp_code);

  ezm_pp_action_t action = EZM_PP_NO_ACTION;
  *comp_code = EZM_PP_ERROR_UNSUPPORTED_CMD;

  return action;
}

static ezm_pp_action_t pp_pkt_processor_get_storage_info(
    ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver, ezm_pp_pkt_t *rx_pkt,
    ezm_pp_pkt_t *tx_pkt, ezm_pp_cmd_comp_code_t *comp_code) {
  EZM_PP_PKT_PROCESSOR_CHECK_PROCESS_CMD_ARG(receiver, sender, rx_pkt, tx_pkt,
                                             comp_code);

  ezm_pp_action_t action = EZM_PP_NO_ACTION;
  *comp_code = EZM_PP_SUCCESS;

  ezm_pp_ep_drv_get_storage_info(rx_pkt, tx_pkt);

  return action;
}

static ezm_pp_action_t pp_pkt_processor_get_img_info(ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver,
                                                    ezm_pp_pkt_t *rx_pkt, ezm_pp_pkt_t *tx_pkt, 
                                                    ezm_pp_cmd_comp_code_t *comp_code) {
  EZM_PP_PKT_PROCESSOR_CHECK_PROCESS_CMD_ARG(receiver, sender, rx_pkt, tx_pkt,
                                             comp_code);

  ezm_pp_action_t action = EZM_PP_NO_ACTION;
  *comp_code = EZM_PP_SUCCESS;

  ezm_pp_ep_drv_get_img_info(rx_pkt, tx_pkt);

  return action;
}
static ezm_pp_action_t
pp_pkt_processor_download(ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver,
                          ezm_pp_pkt_t *rx_pkt, ezm_pp_pkt_t *tx_pkt,
                          ezm_pp_cmd_comp_code_t *comp_code) {
  EZM_PP_PKT_PROCESSOR_CHECK_PROCESS_CMD_ARG(receiver, sender, rx_pkt, tx_pkt,
                                             comp_code);

  ezm_pp_action_t action = EZM_PP_NO_ACTION;
  *comp_code = EZM_PP_SUCCESS;

  if (receiver->role == EZM_PP_OBJ_ROLE_HOST) {
    *comp_code = EZM_PP_ERROR_UNSUPPORTED_CMD;
  } else {
    uint32_t block_size = EZM_PP_MAX_BLOCK_SIZE;
    tx_pkt->msg.download_res.block_size = block_size;
    tx_pkt->msg.download_res.block_number =
        (rx_pkt->msg.download_req.img_size + block_size - 1) / block_size;
    
    int rt = ezm_pp_ep_drv_prepare_endpoint_download(rx_pkt->msg.download_req.img_idx, 
      rx_pkt->msg.download_req.img_size, tx_pkt->msg.download_res.block_size);

    action = EZM_PP_ACTION_REQ_NEXT_BLOCK;
    if(rt != 0) {
      *comp_code = EZM_PP_ERROR_INVALID_DATA;
    }
  }

  return action;
} 


static ezm_pp_action_t
pp_pkt_processor_get_block(ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver,
                           ezm_pp_pkt_t *rx_pkt, ezm_pp_pkt_t *tx_pkt,
                           ezm_pp_cmd_comp_code_t *comp_code) {
  EZM_PP_PKT_PROCESSOR_CHECK_PROCESS_CMD_ARG(receiver, sender, rx_pkt, tx_pkt,
                                             comp_code);

  ezm_pp_action_t action = EZM_PP_NO_ACTION;
  *comp_code = EZM_PP_ERROR_UNSUPPORTED_CMD;
  
  if (receiver->role == EZM_PP_OBJ_ROLE_HOST) {
    // If the receiver is a host, send the block.
    *comp_code = EZM_PP_SUCCESS;
    action = ezm_pp_ep_drv_get_download_block(rx_pkt, tx_pkt);
    if(action == EZM_PP_ACTION_HOST_END_DOWNLOAD) {
      *comp_code = EZM_PP_ERROR_NO_RESPONSE;
    } else if(action == EZM_PP_ACTION_HOST_ERROR_DOWNLOAD) {
      *comp_code = EZM_PP_ERROR_NO_RESPONSE;
    }
  } else {
    *comp_code = EZM_PP_ERROR_UNSUPPORTED_CMD;
  }

  return action;
}

static ezm_pp_action_t
pp_pkt_processor_boot_img(ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver,
                          ezm_pp_pkt_t *rx_pkt, ezm_pp_pkt_t *tx_pkt,
                          ezm_pp_cmd_comp_code_t *comp_code) {
  EZM_PP_PKT_PROCESSOR_CHECK_PROCESS_CMD_ARG(receiver, sender, rx_pkt, tx_pkt,
                                             comp_code);

  ezm_pp_action_t action = EZM_PP_NO_ACTION;
  *comp_code = EZM_PP_ERROR_UNSUPPORTED_CMD;

  if (receiver->role == EZM_PP_OBJ_ROLE_ENDPOINT) {
      int rt = ezm_pp_ep_drv_set_img_boot(rx_pkt->msg.boot_req.img_index);
      if(rt != 0) {
        *comp_code = EZM_PP_ERROR_INVALID_DATA;
      } else {
        *comp_code = EZM_PP_SUCCESS;
        action = EZM_PP_ACTION_BOOT_IMG;
      }
  }

  return action;
}

static ezm_pp_action_t pp_pkt_processor_remove_img(
    ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver, ezm_pp_pkt_t *rx_pkt,
    ezm_pp_pkt_t *tx_pkt, ezm_pp_cmd_comp_code_t *comp_code) {
  EZM_PP_PKT_PROCESSOR_CHECK_PROCESS_CMD_ARG(receiver, sender, rx_pkt, tx_pkt,
                                             comp_code);

  ezm_pp_action_t action = EZM_PP_NO_ACTION;
  *comp_code = EZM_PP_ERROR_UNSUPPORTED_CMD;

  ezm_pp_ep_drv_remove_img(rx_pkt->msg.remove_img_req.img_index);
  *comp_code = EZM_PP_SUCCESS;

  return action;
}

static ezm_pp_action_t pp_pkt_processor_retransmit(
    ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver, ezm_pp_pkt_t *rx_pkt,
    ezm_pp_pkt_t *tx_pkt, ezm_pp_cmd_comp_code_t *comp_code) {
  EZM_PP_PKT_PROCESSOR_CHECK_PROCESS_CMD_ARG(receiver, sender, rx_pkt, tx_pkt,
                                             comp_code);

  ezm_pp_action_t action = EZM_PP_NO_ACTION;
  *comp_code = EZM_PP_ERROR_UNSUPPORTED_CMD;

  // TODO: Implement actual retransmit logic.

  return action;
}

static ezm_pp_action_t pp_pkt_processor_get_version_res(
    ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver, ezm_pp_pkt_t *rx_pkt,
    ezm_pp_pkt_t *tx_pkt, ezm_pp_cmd_comp_code_t *comp_code) {
  EZM_PP_PKT_PROCESSOR_CHECK_PROCESS_CMD_ARG(receiver, sender, rx_pkt, tx_pkt,
                                             comp_code);

  ezm_pp_action_t action = EZM_PP_ACTION_NO_RESPONSE;
  *comp_code = EZM_PP_SUCCESS;

  if( receiver->role == EZM_PP_OBJ_ROLE_HOST) {
    print_serial_log("PP Version: %d.%d.%d\n", EZM_PP_VERSION.major,
                           EZM_PP_VERSION.minor, EZM_PP_VERSION.patch);
    action = EZM_PP_ACTION_IDENT_CAPABILITIES;
  }

  return action;
}

static ezm_pp_action_t pp_pkt_processor_get_capabilities_res(
    ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver, ezm_pp_pkt_t *rx_pkt,
    ezm_pp_pkt_t *tx_pkt, ezm_pp_cmd_comp_code_t *comp_code) {
  EZM_PP_PKT_PROCESSOR_CHECK_PROCESS_CMD_ARG(receiver, sender, rx_pkt, tx_pkt,
                                             comp_code);

  ezm_pp_action_t action = EZM_PP_ACTION_NO_RESPONSE;
  *comp_code = EZM_PP_SUCCESS;

  if( receiver->role == EZM_PP_OBJ_ROLE_HOST) {
    print_serial_log("PP Capabilities: Max Data Transfer: %d, Max Message Size: %d\n",
      rx_pkt->msg.get_capabilities_res.max_data_transfer,
      rx_pkt->msg.get_capabilities_res.max_message_size);
      action = EZM_PP_ACTION_IDENT_STORAGE_INFO;
  }

  // TODO: Process capabilities response if necessary.

  return action;
}

static ezm_pp_action_t pp_pkt_processor_set_capabilities_res(
    ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver, ezm_pp_pkt_t *rx_pkt,
    ezm_pp_pkt_t *tx_pkt, ezm_pp_cmd_comp_code_t *comp_code) {
  EZM_PP_PKT_PROCESSOR_CHECK_PROCESS_CMD_ARG(receiver, sender, rx_pkt, tx_pkt,
                                             comp_code);

  ezm_pp_action_t action = EZM_PP_ACTION_NO_RESPONSE;
  *comp_code = EZM_PP_ERROR_UNSUPPORTED_CMD;

  // TODO: Process set_capabilities response if necessary.

  return action;
}

static ezm_pp_action_t pp_pkt_processor_get_storage_info_res(
    ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver, ezm_pp_pkt_t *rx_pkt,
    ezm_pp_pkt_t *tx_pkt, ezm_pp_cmd_comp_code_t *comp_code) {
  EZM_PP_PKT_PROCESSOR_CHECK_PROCESS_CMD_ARG(receiver, sender, rx_pkt, tx_pkt,
                                             comp_code);

  ezm_pp_action_t action = EZM_PP_ACTION_NO_RESPONSE;
  *comp_code = EZM_PP_SUCCESS;

  if( receiver->role == EZM_PP_OBJ_ROLE_HOST) {
    print_serial_log("Storage Info: Total Size: %d bytes, Img Mask: %x\n",
      rx_pkt->msg.get_storage_info_res.storage_size,
      rx_pkt->msg.get_storage_info_res.img_mask);
    action = EZM_PP_NO_ACTION;
  }

  // TODO: Process storage info response if necessary.

  return action;
}

static ezm_pp_action_t pp_pkt_processor_get_img_info_res(
    ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver, ezm_pp_pkt_t *rx_pkt,
    ezm_pp_pkt_t *tx_pkt, ezm_pp_cmd_comp_code_t *comp_code) {
  EZM_PP_PKT_PROCESSOR_CHECK_PROCESS_CMD_ARG(receiver, sender, rx_pkt, tx_pkt,
                                             comp_code);

  ezm_pp_action_t action = EZM_PP_ACTION_NO_RESPONSE;
  *comp_code = EZM_PP_SUCCESS;

  if( receiver->role == EZM_PP_OBJ_ROLE_HOST) {
    action = EZM_PP_NO_ACTION;
    if(rx_pkt->msg.get_img_info_res.ctrl_hdr.cmd_res != EZM_PP_SUCCESS) {
      print_serial_log("Image Info: Error retrieving image info, Code: %d",
        rx_pkt->msg.get_img_info_res.ctrl_hdr.cmd_res);
      return action;
    }
    if(rx_pkt->msg.get_img_info_res.img_status == 0xFFFF) {
      print_serial_log("Image Info: Invalid Image Index | Unused");
    } else {
      print_serial_log("Image [%d] info retrieved:", rx_pkt->msg.get_img_info_res.img_status);
      print_serial_log(" Type: %d", rx_pkt->msg.get_img_info_res.img_type);
      print_serial_log(" Size: %d bytes", rx_pkt->msg.get_img_info_res.img_size);
      print_serial_log(" Version: %d.%d.%d.%d",
        rx_pkt->msg.get_img_info_res.img_version[0],
        rx_pkt->msg.get_img_info_res.img_version[1],
        rx_pkt->msg.get_img_info_res.img_version[2],
        rx_pkt->msg.get_img_info_res.img_version[3]);
    }
  }

  // TODO: Process img info response if necessary.

  return action;
}

static ezm_pp_action_t pp_pkt_processor_download_res(
    ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver, ezm_pp_pkt_t *rx_pkt,
    ezm_pp_pkt_t *tx_pkt, ezm_pp_cmd_comp_code_t *comp_code) {
  EZM_PP_PKT_PROCESSOR_CHECK_PROCESS_CMD_ARG(receiver, sender, rx_pkt, tx_pkt,
                                             comp_code);

  ezm_pp_action_t action = EZM_PP_ACTION_NO_RESPONSE;
  *comp_code = EZM_PP_SUCCESS;

  // TODO: Process download response if necessary.

  return action;
}

static ezm_pp_action_t pp_pkt_processor_get_block_res(
    ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver, ezm_pp_pkt_t *rx_pkt,
    ezm_pp_pkt_t *tx_pkt, ezm_pp_cmd_comp_code_t *comp_code) {
  EZM_PP_PKT_PROCESSOR_CHECK_PROCESS_CMD_ARG(receiver, sender, rx_pkt, tx_pkt,
                                             comp_code);

  ezm_pp_action_t action = EZM_PP_NO_ACTION;
  *comp_code = EZM_PP_SUCCESS;

  if (receiver->role == EZM_PP_OBJ_ROLE_ENDPOINT) {
    action = ezm_pp_ep_drv_write_block(rx_pkt->msg.get_block_res.block_data, rx_pkt->msg.get_block_res.block_size);
  }

  return action;
}

static ezm_pp_action_t pp_pkt_processor_boot_img_res(
    ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver, ezm_pp_pkt_t *rx_pkt,
    ezm_pp_pkt_t *tx_pkt, ezm_pp_cmd_comp_code_t *comp_code) {
  EZM_PP_PKT_PROCESSOR_CHECK_PROCESS_CMD_ARG(receiver, sender, rx_pkt, tx_pkt,
                                             comp_code);

  ezm_pp_action_t action = EZM_PP_NO_ACTION;
  *comp_code = EZM_PP_SUCCESS;

  if(receiver->role == EZM_PP_OBJ_ROLE_HOST) {
    print_serial_log("Download parameters received:");
    print_serial_log("  Blocks: %d", rx_pkt->msg.download_res.block_number);
    print_serial_log("  Blocks size: %d", rx_pkt->msg.download_res.block_size);
    action = EZM_PP_ACTION_WAIT_FOR_NEXT_BLOCK;
  }

  return action;
}

static ezm_pp_action_t pp_pkt_processor_remove_img_res(
    ezm_pp_obj_t *sender, ezm_pp_obj_t *receiver, ezm_pp_pkt_t *rx_pkt,
    ezm_pp_pkt_t *tx_pkt, ezm_pp_cmd_comp_code_t *comp_code) {
  EZM_PP_PKT_PROCESSOR_CHECK_PROCESS_CMD_ARG(receiver, sender, rx_pkt, tx_pkt,
                                             comp_code);

  ezm_pp_action_t action = EZM_PP_NO_ACTION;
  *comp_code = EZM_PP_ERROR_UNSUPPORTED_CMD;

  return action;
}

ezm_pp_action_t ezm_pp_pkt_processor_message(ezm_pp_obj_t *sender,
                                             ezm_pp_obj_t *receiver,
                                             uint32_t *pkt_length,
                                             ezm_pp_pkt_t *rx_pkt,
                                             ezm_pp_pkt_t *tx_pkt) {
  if (rx_pkt == NULL || tx_pkt == NULL || pkt_length == NULL ||
      sender == NULL || receiver == NULL) {
    return EZM_PP_ACTION_NO_RESPONSE;
  }

  ezm_pp_action_t action = EZM_PP_ACTION_DISCARD_INVALID_PACKET;
  ezm_pp_cmd_comp_code_t completion_code;

  // For now only support complete messages.
  if (rx_pkt->tp_hdr.som != 1 && rx_pkt->tp_hdr.eom != 1) {
    return EZM_PP_ACTION_NO_RESPONSE;
  }

  // Only process Control messages.
  if (rx_pkt->msg.ctrl_msg_req_gen.cmn_hdr.msg_type != EZM_PP_MSG_TYPE_CTRL) {
    return EZM_PP_ACTION_DISCARD_INVALID_PACKET;
  }

  ezm_pp_cmd_code_t cmd_code = rx_pkt->msg.ctrl_msg_req_gen.cmd_code;
  if (rx_pkt->msg.ctrl_msg_req_gen.rq) {
    // Process requests.
    if (cmd_code < EZM_PP_CMD_MAX && pp_process_cmd[cmd_code] != NULL) {
      if (*pkt_length != ezm_pp_pkt_maker_req_len(rx_pkt, cmd_code)) {
        return action;
      }
      action = pp_process_cmd[cmd_code](sender, receiver, rx_pkt, tx_pkt,
                                        &completion_code);
    } else {
      action = EZM_PP_NO_ACTION;
      completion_code = EZM_PP_ERROR_UNSUPPORTED_CMD;
    }

    if (completion_code != EZM_PP_ERROR_NO_RESPONSE) {
      *pkt_length = ezm_pp_pkt_maker_res(receiver, sender, tx_pkt, cmd_code, completion_code);
    } else {
      *pkt_length = 0;
    }
  } else {
    // Process responses.
    if (cmd_code < EZM_PP_CMD_MAX && pp_process_cmd_res[cmd_code] != NULL) {
      if (*pkt_length !=
          ezm_pp_pkt_maker_res_len(rx_pkt, cmd_code,
                                   rx_pkt->msg.ctrl_msg_res_gen.cmd_res)) {
        return action;
      }
      action = pp_process_cmd_res[cmd_code](sender, receiver, rx_pkt, tx_pkt,
                                            &completion_code);
    } else {
      completion_code = EZM_PP_ERROR_UNSUPPORTED_CMD;
    }

    // Responses do not generate further responses.
    *pkt_length = 0;
  }

  return action;
}