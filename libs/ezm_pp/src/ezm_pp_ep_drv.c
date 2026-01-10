#include <string.h>

#include "ezm_ut_utils.h"

#include "ezm_pp_ep_drv.h"
#ifndef CONFIG_HOST_BUILD
#include "ezm_flash_admin.h"
#include "ezm_flash_bl.h"
#endif

STATIC_GLOBAL ezm_pp_ep_download_op_t download_op;

static void ezm_pp_ep_drv_close_download_partition(void)
{
#ifndef CONFIG_HOST_BUILD
    if(download_op.src_handle != NULL) {
        ezm_flash_admin_close_partition((ezm_flash_partition_handler_t)download_op.src_handle);
        download_op.src_handle = NULL;
    }
#else
    return;
#endif
}

static void ezm_pp_ep_drv_reset_download_info(void)
{
    download_op.ez_img = NULL;
    download_op.ez_img_size = 0; 
    download_op.ez_img_offset = 0;
}

static void ezm_pp_ep_drv_reset_download_op_info(void)
{
    download_op.block_size = 0;
    download_op.total_blocks = 0;
    download_op.src_handle = NULL;
}

static void ezm_pp_ep_drv_set_download_op_info(uint32_t block_size)
{
    download_op.block_size = block_size;
    download_op.total_blocks = (download_op.ez_img_size + block_size - 1) / block_size;
}


static void ezm_pp_ep_drv_set_download_img_info(uint8_t * ez_img, uint32_t ez_img_size)
{
    download_op.ez_img = ez_img;
    download_op.ez_img_size = ez_img_size; 
    download_op.ez_img_offset = 0;
}

void ezm_pp_ep_drv_end_download(void)
{
    ezm_pp_ep_drv_close_download_partition();
    ezm_pp_ep_drv_reset_download_info();
    ezm_pp_ep_drv_reset_download_op_info();
}

ezm_pp_action_t ezm_pp_ep_drv_get_download_block(ezm_pp_pkt_t * rx_pkt, ezm_pp_pkt_t * tx_pkt)
{
    (void)rx_pkt;
    ezm_pp_action_t action = EZM_PP_ACTION_WAIT_FOR_NEXT_BLOCK;

    if(rx_pkt->msg.get_block_req.block_number == 0xFFFFFFFF) {
        // End of download indication
        ezm_pp_ep_drv_end_download();
        action = EZM_PP_ACTION_HOST_END_DOWNLOAD;
        tx_pkt->msg.get_block_res.block_size = 0;
        return action;
    } else if(rx_pkt->msg.get_block_req.block_number == 0xDEADBEEF) {
        // Invalid write indication
        ezm_pp_ep_drv_end_download();
        action = EZM_PP_ACTION_HOST_ERROR_DOWNLOAD;
        tx_pkt->msg.get_block_res.block_size = 0;
        return action;
    } else if(rx_pkt->msg.get_block_req.block_number != 
              (download_op.ez_img_offset + download_op.block_size - 1) / download_op.block_size) {
        // Invalid block number requested
        action = EZM_PP_ACTION_HOST_ERROR_DOWNLOAD;
        tx_pkt->msg.get_block_res.block_size = 0;
        return action;
    } else {
        uint8_t * block_buffer = tx_pkt->msg.get_block_res.block_data;
    
        if(download_op.ez_img == NULL || download_op.ez_img_offset >= download_op.ez_img_size) {
            tx_pkt->msg.get_block_res.block_size = 0;
            return EZM_PP_ACTION_HOST_ERROR_DOWNLOAD;
        }
    
        uint32_t remaining_size = download_op.ez_img_size - download_op.ez_img_offset;
        tx_pkt->msg.get_block_res.block_size = (download_op.block_size < remaining_size) ? download_op.block_size : remaining_size;
    
        memcpy(block_buffer, download_op.ez_img + download_op.ez_img_offset, tx_pkt->msg.get_block_res.block_size);
        tx_pkt->msg.get_block_res.block_number = (download_op.ez_img_offset + download_op.block_size - 1) / download_op.block_size;
        download_op.ez_img_offset += tx_pkt->msg.get_block_res.block_size;
    }
    return action;
}

int ezm_pp_ep_drv_set_img_boot(uint8_t img_index)
{
    #ifndef CONFIG_HOST_BUILD
    int rt = ezm_flash_admin_set_boot_partition(img_index);
    return rt;
    #else
    return 0;
    #endif
}

void ezm_pp_ep_drv_remove_img(uint8_t img_index)
{
#ifndef CONFIG_HOST_BUILD
    ezm_flash_admin_erase_partition(img_index);
#else
    return;
#endif
}

void ezm_pp_ep_drv_get_storage_info(ezm_pp_pkt_t * rx_pkt, ezm_pp_pkt_t * tx_pkt)
{
#ifndef CONFIG_HOST_BUILD
    (void)rx_pkt;
    tx_pkt->msg.get_storage_info_res.storage_size = ezm_flash_admin_get_memory_size();
    tx_pkt->msg.get_storage_info_res.img_mask = ezm_flash_admin_get_partitions_mask();
#else
    return;
#endif
} 

uint32_t ezm_pp_ep_drv_get_img_size(void) {
    return download_op.ez_img_size;
}

uint32_t ezm_pp_ep_drv_get_block_size(void) {
    return download_op.block_size;
}

int ezm_pp_ep_drv_prepare_host_download(uint8_t * img, uint32_t img_size, uint32_t block_size){
    ezm_pp_ep_drv_set_download_img_info(img, img_size);
    ezm_pp_ep_drv_set_download_op_info(block_size);
    return 0;
}

int ezm_pp_ep_drv_prepare_endpoint_download(uint8_t img_idx, uint32_t img_size, uint32_t block_size){
#ifndef CONFIG_HOST_BUILD
    ezm_pp_ep_drv_set_download_img_info(NULL, img_size);
    ezm_pp_ep_drv_set_download_op_info(block_size);

    int rt = ezm_flash_admin_open_partition((ezm_flash_partition_handler_t *)&download_op.src_handle, img_idx, EZM_FLASH_ADMIN_OPEN_MODE_WRITE, img_size);
    return (rt);
#else
    return 0;
#endif
}

ezm_pp_action_t ezm_pp_ep_drv_write_block(uint8_t * data, uint32_t size) {
#ifndef CONFIG_HOST_BUILD
    ezm_pp_action_t action = EZM_PP_ACTION_INVALID_WRITE;
    int rt = ezm_flash_admin_write_partition((ezm_flash_partition_handler_t)download_op.src_handle, data, size);
    if(rt == EZM_FLASH_ADMIN_OK) {
        action = EZM_PP_ACTION_REQ_NEXT_BLOCK;
        download_op.ez_img_offset += size;
        if(download_op.ez_img_offset == download_op.ez_img_size) {
            action = EZM_PP_ACTION_END_DOWNLOAD;
        }
    }
    return action;
#else
    return EZM_PP_ACTION_INVALID_WRITE;
#endif
}

uint32_t ezm_pp_ep_drv_get_next_block_number(void) {
    return ((download_op.ez_img_offset + download_op.block_size - 1) / download_op.block_size);
}

void ezm_pp_ep_drv_get_img_info(ezm_pp_pkt_t * rx_pkt, ezm_pp_pkt_t * tx_pkt)
{
#ifndef CONFIG_HOST_BUILD
    uint8_t img_index = rx_pkt->msg.get_img_info_req.img_index;
    ezm_img_header_t img_header = {0};
    if(ezm_flash_admin_get_partition_info(img_index, &img_header) == EZM_FLASH_ADMIN_OK) {
        tx_pkt->msg.get_img_info_res.img_status = img_index; // Indicate valid image
        memcpy(tx_pkt->msg.get_img_info_res.img_name, img_header.name, EZM_PP_IMG_NAME_SIZE);
        tx_pkt->msg.get_img_info_res.img_size = img_header.img_size;
        memcpy(tx_pkt->msg.get_img_info_res.img_version, &img_header.fw_version, EZM_PP_IMG_VERSION_SIZE);
        tx_pkt->msg.get_img_info_res.img_type = img_header.fw_type;
    } else {
        tx_pkt->msg.get_img_info_res.img_status = 0xFFFF; // Indicate invalid image
        memset(tx_pkt->msg.get_img_info_res.img_name, 0xff, EZM_PP_IMG_NAME_SIZE);
        tx_pkt->msg.get_img_info_res.img_size = 0xffffffff;
        memset(tx_pkt->msg.get_img_info_res.img_version, 0xff, EZM_PP_IMG_VERSION_SIZE);
        tx_pkt->msg.get_img_info_res.img_type = 0xffff;
    }
#else
    return;
#endif
}

int ezm_pp_ep_drv_boot_img(void)
{
#ifndef CONFIG_HOST_BUILD
    int rt = ezm_flash_bl_cpy_img_to_flash();
    if(rt == EZM_FLASH_ADMIN_OK) {
        ezm_flash_bl_boot_flash_img();
    }
    return rt;
#else
    return 0;
#endif
}