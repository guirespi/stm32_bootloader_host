#ifndef EZM_PP_EP_DRV_H_
#define EZM_PP_EP_DRV_H_

#include <stdint.h>
#include <ezm_pp.h>
#include <ezm_pp_pkt.h>

typedef struct {
    uint8_t * ez_img;
    void * src_handle;
    uint32_t ez_img_size;
    uint32_t ez_img_offset;
    uint32_t block_size;
    uint32_t total_blocks;
} ezm_pp_ep_download_op_t;

void ezm_pp_ep_drv_get_storage_info(ezm_pp_pkt_t * rx_pkt, ezm_pp_pkt_t * tx_pkt);

void ezm_pp_ep_drv_get_img_info(ezm_pp_pkt_t * rx_pkt, ezm_pp_pkt_t * tx_pkt);

void ezm_pp_ep_drv_end_download(void);

int ezm_pp_ep_drv_set_img_boot(uint8_t img_index);

void ezm_pp_ep_drv_remove_img(uint8_t img_index);

ezm_pp_action_t ezm_pp_ep_drv_get_download_block(ezm_pp_pkt_t * rx_pkt, ezm_pp_pkt_t * tx_pkt);

int ezm_pp_ep_drv_prepare_host_download(uint8_t * img, uint32_t img_size, uint32_t block_size);

uint32_t ezm_pp_ep_drv_get_img_size(void);

uint32_t ezm_pp_ep_drv_get_block_size(void);

int ezm_pp_ep_drv_prepare_endpoint_download(uint8_t img_idx, uint32_t img_size, uint32_t block_size);

ezm_pp_action_t ezm_pp_ep_drv_write_block(uint8_t * data, uint32_t size);

uint32_t ezm_pp_ep_drv_get_next_block_number(void);

int ezm_pp_ep_drv_boot_img(void);

#endif /* EZM_PP_EP_DRV_H_ */