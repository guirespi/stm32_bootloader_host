/*
 * app_bootloader_command.h
 *
 *  Created on: Aug 11, 2024
 *      Author: guirespi
 */

#ifndef APPLICATION_APP_BOOTLOADER_INC_APP_BOOTLOADER_COMMAND_H_
#define APPLICATION_APP_BOOTLOADER_INC_APP_BOOTLOADER_COMMAND_H_

#include <stdint.h>

#define APP_BOOTLOADER_CMD_MAGIC_BYTE (0xAA)

typedef enum
{
	APP_BOOTLOADER_CMD_HELLO = 0, /*< Client hello to host */
	APP_BOOTLOADER_CMD_HOST_HELLO, /*< Host helllo to client */

	/*< Commands related to download process */

	APP_BOOTLOADER_CMD_DOWNLOAD_REQ, /*< Host download request */
	APP_BOOTLOADER_CMD_DOWNLOAD_PARAM_REQ, /*< Client download parameters request */
	APP_BOOTLOADER_CMD_DOWNLOAD_PARAM_RES, /*< Host download parameters response */
	APP_BOOTLOADER_CMD_DOWNLOAD_BLOCK_REQ, /*< Client download block request */
	APP_BOOTLOADER_CMD_DOWNLOAD_BLOCK_RES, /*< Host download block response */
	APP_BOOTLOADER_CMD_DOWNLOAD_END, /*< Client download end signal */

	/**< Commands to boot applications */
	APP_BOOTLOADER_CMD_BOOT_APP,

	/*< Commands related to partition information */

	APP_BOOTLOADER_CMD_PART_INFO_REQ,
	APP_BOOTLOADER_CMD_PART_INFO_RES,

	/*< Command for error message */
	APP_BOOTLOADER_CMD_ERROR,

	APP_BOOTLOADER_CMD_MAX, /*< Boundary of available commands */
}app_bootloader_command;

/* Command's content*/

typedef struct __attribute__((packed))
{
	uint8_t 	part_nbr;
	uint32_t 	binary_size;
}app_bootloader_cmd_dl_req;

typedef struct __attribute__((packed))
{
	uint8_t 	type;
	uint32_t 	block_size;
}app_bootloader_cmd_dl_param_req;

typedef enum __attribute__((packed))
{
	APP_BOOTLOADER_DL_RAW = 0,
	APP_BOOTLOADER_DL_COMPRESS,
}app_bootloder_dl_type;

typedef struct __attribute__((packed))
{
	uint8_t 	type;
	uint32_t 	total_block_nbr;
	uint32_t 	block_size;
}app_bootloader_cmd_dl_param_res;

typedef struct __attribute__((packed))
{
	uint32_t block_nbr;
}app_bootloader_cmd_dl_block_req;

typedef struct __attribute__((packed))
{
	uint32_t block_nbr;
	uint32_t data_size;
	uint8_t  data[];
}app_bootloader_cmd_dl_block_res;

typedef struct __attribute__((packed))
{
	uint8_t error;
	char  error_msg[];
}app_bootloader_cmd_err;

typedef struct __attribute__((packed))
{
	uint8_t partition_nbr;
}app_bootloader_cmd_boot_app;

typedef enum __attribute__((packed))
{
	APP_BOOTLOADER_CMD_OK = 0,
	APP_BOOTLOADER_CMD_E_MEM,
	APP_BOOTLOADER_CMD_E_UNKNOWN,
	APP_BOOTLOADER_CMD_E_PARAM,
	APP_BOOTLOADER_CMD_E_NULL,
	APP_BOOTLOADER_CMD_E_SIZE,
	APP_BOOTLOADER_CMD_E_INVALID,
	APP_BOOTLOADER_CMD_E_FAIL,
}app_bootloader_command_err_t;

typedef struct __attribute__((packed))
{
	uint8_t magic;
	uint8_t command;
	uint16_t total_length;
	uint8_t data[];
}app_bootloader_frame_t;

typedef struct
{
	uint8_t * frame;
	uint16_t frame_size;
}app_bootloader_build_res_t;

int app_bootloader_build_hello(app_bootloader_build_res_t * build_digest);

int app_bootloader_build_host_hello(app_bootloader_build_res_t * build_digest);

int app_bootloader_build_dl_req(app_bootloader_build_res_t * build_digest, uint8_t partition_nbr, uint32_t binary_size);

int app_bootloader_build_dl_param_req(app_bootloader_build_res_t * build_digest, uint8_t type, uint16_t block_size);

int app_bootloader_build_dl_param_res(app_bootloader_build_res_t * build_digest, uint8_t type, uint32_t total_block_nbr, uint16_t block_size);

int app_bootloader_build_dl_block_req(app_bootloader_build_res_t * build_digest, uint32_t block_nbr);

int app_bootloader_build_dl_block_res(app_bootloader_build_res_t * build_digest, uint32_t block_nbr, uint32_t data_size, uint8_t * data);

int app_bootloader_build_dl_end(app_bootloader_build_res_t * build_digest);

int app_bootloader_build_boot_app(app_bootloader_build_res_t * build_digest, uint8_t partition_nbr);

int app_bootloader_build_error(app_bootloader_build_res_t * build_digest, uint8_t error, char * message);

int app_bootloader_command_check(uint8_t * buffer, uint16_t buffer_size, app_bootloader_frame_t ** command_digest);

#endif /* APPLICATION_APP_BOOTLOADER_INC_APP_BOOTLOADER_COMMAND_H_ */
