#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "stm32f4xx_hal.h"

#include "app_bootloader.h"
#include "app_bootloader_command.h"
#include "API_console.h"
#include "API_spi_flash.h"

#include "API_log.h"
#define tag "app_bootloader.c"
#define print_serial_info(format, ...) LOG_LEVEL(LOG_INFO, tag, format, ##__VA_ARGS__)
#define print_serial_warn(format, ...) LOG_LEVEL(LOG_WARN, tag, format, ##__VA_ARGS__)
#define print_serial_error(format, ...) LOG_LEVEL(LOG_ERROR, tag, format, ##__VA_ARGS__)
#define print_serial_hex(data, data_size) LOG_HEXDUMP(tag, data, data_size, LOG_WARN)

#define APP_BOOTLOADER_BUFFER_SIZE (5120)
#define APP_BOOTLOADER_DEFAULT_BLOCK_SIZE (4096)
#define APP_BOOTLOADER_DEFAULT_DL_TYPE (APP_BOOTLOADER_DL_RAW)
#define APP_BOOTLOADER_DEFAULT_PARTITION_SIZE (0x50000) /* 327 kB */

#define APP_BOOTLOADER_PARTITION_MAGIC_BYTE		(0x2609)
#define APP_BOOTLOADER_PARTITION_FLAG_COMPLETE 	(1<<0)
#define APP_BOOTLOADER_PARTITION_HEADER_MAX_SIZE (256)

#define BOOTLOADER_ADDR (0x8000000)
#define APP_ADDR		(0x8080000)

typedef void (*jump_function)(void);

typedef struct
{
	uint32_t actual_size;
	uint32_t total_size;
	uint32_t total_block_nbr;
	uint32_t actual_block_nbr;
	uint32_t block_size;
	app_bootloder_dl_t dl_type;
	uint8_t partition_nbr;
}app_bootloader_dl_t;

typedef struct
{
	app_bootloader_state_t state;
	app_bootloader_dl_t dl_status;
}app_bootloader_t;

typedef struct
{
	uint8_t partition_nbr;
	uint32_t offset;
	uint32_t size;
}app_bootloader_partition_t;

typedef struct __attribute__((packed))
{
	uint16_t magic_byte;
	uint32_t size;
	uint32_t flag;
}app_bootloader_partition_info_t;

static volatile app_bootloader_t app_bootloader = {.state = APP_BOOTLOADER_STATE_DISABLE};
static uint8_t app_bootloader_buffer[APP_BOOTLOADER_BUFFER_SIZE] = {0};

/* The application will have the available partitions, we save the offset, maximum size and partition number or ID*/
static const app_bootloader_partition_t partition_array[] =
{
	{.partition_nbr = 0, .offset = 0		, .size = APP_BOOTLOADER_DEFAULT_PARTITION_SIZE},
	{.partition_nbr = 1, .offset = 0x50000	, .size = APP_BOOTLOADER_DEFAULT_PARTITION_SIZE},
	{.partition_nbr = 2, .offset = 0xA0000	, .size = APP_BOOTLOADER_DEFAULT_PARTITION_SIZE},
};

#define partition_array_size (sizeof(partition_array)/sizeof(partition_array[0]))

void bootloader_boot(uint32_t boot_address)
{
	uint32_t jump_address;
	jump_function jump_to_app = NULL;
	if(((*(uint32_t *) boot_address) & (0x2FF00000)) == 0x20000000)
	{
		print_serial_info("Application found! Booting...");
		jump_address = *((volatile uint32_t *)(boot_address + 4));
		jump_to_app = (jump_function) jump_address;
		__set_MSP(*(uint32_t *) boot_address);
		jump_to_app();
	}
	else
	{
		print_serial_warn("No application found in 0x%x", boot_address);
	}
}

static uint32_t app_bootloader_get_partition_size(uint8_t partition_number)
{
	uint32_t size = 0;
	for(uint8_t i = 0; i < partition_array_size; i++)
	{
		if(partition_number == partition_array[i].partition_nbr)
		{
			size = partition_array[i].size;
			break;
		}
	}
	return size;
}

static int app_bootloader_get_partition_offset(uint8_t partition_number)
{
	int offset = -1;
	for(uint8_t i = 0; i < partition_array_size; i++)
	{
		if(partition_number == partition_array[i].partition_nbr)
		{
			offset = partition_array[i].offset;
			break;
		}
	}
	return offset;
}

static inline void app_bootloader_set_state(app_bootloader_state_t new_state)
{
	app_bootloader.state = new_state;
}

static inline volatile app_bootloader_state_t app_bootloader_get_state(void)
{
	return app_bootloader.state;
}

static int app_bootloader_process_command(app_bootloader_frame_t * command_digest, app_bootloader_build_res_t * build_digest)
{
	int rt =  APP_BOOTLOADER_OK;
	switch((app_bootloader_command)command_digest->command)
	{
		case APP_BOOTLOADER_CMD_HOST_HELLO:
		{
			app_bootloader_set_state(APP_BOOTLOADER_STATE_INIT);
			break;
		}
		case APP_BOOTLOADER_CMD_DOWNLOAD_REQ:
		{
			print_serial_info("Download request received");
			/* Our download only allows raw type for now */
			app_bootloader_cmd_dl_req * dl_req = (app_bootloader_cmd_dl_req *)command_digest->data;

			uint32_t size = app_bootloader_get_partition_size(dl_req->part_nbr);
			if(size == 0)
			{
				rt = app_bootloader_build_error(build_digest, APP_BOOTLOADER_CMD_E_UNKNOWN, "Not declared partition");
				break;
			}
			if(size < dl_req->binary_size)
			{
				rt = app_bootloader_build_error(build_digest, APP_BOOTLOADER_CMD_E_SIZE, "File does not fit in partition");
				break;
			}

			app_bootloader.dl_status.total_size = dl_req->binary_size;
			app_bootloader.dl_status.partition_nbr = dl_req->part_nbr;

			rt = app_bootloader_build_dl_param_req(build_digest, APP_BOOTLOADER_DEFAULT_DL_TYPE, APP_BOOTLOADER_DEFAULT_BLOCK_SIZE);
			break;
		}
		case APP_BOOTLOADER_CMD_DOWNLOAD_PARAM_RES:
		{
			print_serial_info("Download parameter response received");
			app_bootloader_cmd_dl_param_res * dl_param_res =  (app_bootloader_cmd_dl_param_res *)command_digest->data;

			uint32_t partition_size = app_bootloader_get_partition_size(app_bootloader.dl_status.partition_nbr);
			uint32_t partition_offset = app_bootloader_get_partition_offset(app_bootloader.dl_status.partition_nbr);

			app_bootloader.dl_status.dl_type = dl_param_res->type;
			app_bootloader.dl_status.block_size = dl_param_res->block_size;
			app_bootloader.dl_status.total_block_nbr = dl_param_res->total_block_nbr;
			app_bootloader.dl_status.actual_block_nbr = 0;
			app_bootloader.dl_status.actual_size = 0;

			rt = spi_flash_erase_range(partition_offset, partition_size);
			if(rt != SPI_FLASH_OK)
			{
				rt = app_bootloader_build_error(build_digest, APP_BOOTLOADER_CMD_E_FAIL, "Error erasing partition");
				break;
			}

			app_bootloader_partition_info_t partition_info = {
					.magic_byte = APP_BOOTLOADER_PARTITION_MAGIC_BYTE,
					.size = app_bootloader.dl_status.total_size,
					.flag = 0,
			};

			rt = spi_flash_write((uint8_t *)&partition_info, partition_offset, sizeof(partition_info));
			if(rt != SPI_FLASH_OK)
			{
				rt = app_bootloader_build_error(build_digest, APP_BOOTLOADER_CMD_E_FAIL, "Error preparing partition header");
				break;
			}

			rt = app_bootloader_build_dl_block_req(build_digest, app_bootloader.dl_status.actual_block_nbr);
			break;
		}
		case APP_BOOTLOADER_CMD_DOWNLOAD_BLOCK_RES:
		{
			print_serial_info("Download block response received");
			print_serial_info("Download status [%d/%d]", app_bootloader.dl_status.total_block_nbr, app_bootloader.dl_status.actual_block_nbr);

			app_bootloader_cmd_dl_block_res * dl_block_res =  (app_bootloader_cmd_dl_block_res *)command_digest->data;
			int address = app_bootloader_get_partition_offset(app_bootloader.dl_status.partition_nbr);
			/* We reserve the first page of a partition for partition info like flags, size, etc. This will become
			 * useful when selecting and booting a saved in flash application */
			uint32_t offset = address + APP_BOOTLOADER_PARTITION_HEADER_MAX_SIZE;

			/*Todo: Check data size arrived */

			rt = spi_flash_write(dl_block_res->data, offset + app_bootloader.dl_status.actual_size, dl_block_res->data_size);
			if(rt != SPI_FLASH_OK)
			{
				rt = app_bootloader_build_error(build_digest, APP_BOOTLOADER_CMD_E_FAIL, "Error writing into flash");
				break;
			}
			else
			{
				app_bootloader.dl_status.actual_block_nbr++;
				if(app_bootloader.dl_status.actual_block_nbr == app_bootloader.dl_status.total_block_nbr)
				{
					print_serial_info("Download finish");
					app_bootloader_partition_info_t partition_info = {
							.magic_byte = APP_BOOTLOADER_PARTITION_MAGIC_BYTE,
							.size = app_bootloader.dl_status.total_size,
							.flag = APP_BOOTLOADER_PARTITION_FLAG_COMPLETE,
					};

					uint32_t partition_offset = app_bootloader_get_partition_offset(app_bootloader.dl_status.partition_nbr);
					rt = spi_flash_write((uint8_t *)&partition_info, partition_offset, sizeof(partition_info));
					if(rt != SPI_FLASH_OK)
					{
						rt = app_bootloader_build_error(build_digest, APP_BOOTLOADER_CMD_E_FAIL, "Error updating partition header");
						break;
					}

					rt = app_bootloader_build_dl_end(build_digest);
				}
				else
				{
					app_bootloader.dl_status.actual_size += dl_block_res->data_size;
					rt = app_bootloader_build_dl_block_req(build_digest, app_bootloader.dl_status.actual_block_nbr);
				}
			}
			break;
		}
		case APP_BOOTLOADER_CMD_BOOT_APP:
		{
			int partition_offset = app_bootloader_get_partition_offset(app_bootloader.dl_status.partition_nbr);
			if(partition_offset < 0)
			{
				rt = app_bootloader_build_error(build_digest, APP_BOOTLOADER_CMD_E_INVALID, "Partition does not exist");
				break;
			}

			uint8_t buffer[APP_BOOTLOADER_PARTITION_HEADER_MAX_SIZE] = {0};
			rt = spi_flash_read(buffer, (uint32_t)partition_offset, APP_BOOTLOADER_PARTITION_HEADER_MAX_SIZE);
			if(rt != SPI_FLASH_OK)
			{
				rt = app_bootloader_build_error(build_digest, APP_BOOTLOADER_CMD_E_INVALID, "Error reading SPI flash");
				break;
			}

			app_bootloader_partition_info_t * partition_info = (app_bootloader_partition_info_t *) buffer;

			if(partition_info->magic_byte != APP_BOOTLOADER_PARTITION_MAGIC_BYTE)
			{
				rt = app_bootloader_build_error(build_digest, APP_BOOTLOADER_CMD_E_INVALID, "Invalid partition requested");
				break;
			}
			if((partition_info->flag & (APP_BOOTLOADER_PARTITION_FLAG_COMPLETE)) != APP_BOOTLOADER_PARTITION_FLAG_COMPLETE)
			{
				rt = app_bootloader_build_error(build_digest, APP_BOOTLOADER_CMD_E_INVALID, "Incomplete partition requested");
				break;
			}

			/*To erase APP partition:
			* -> Sector 8: 0x08080000 - 0x080A0000
			* -> Sector 11: 0x80E0000 - 0x080FFFFF
			* We must erase 4 sectors */
			HAL_FLASH_Unlock();
			uint32_t SectorError = 0;
			FLASH_EraseInitTypeDef pEraseInit = {0};
			pEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
			pEraseInit.Sector = FLASH_SECTOR_8;
			pEraseInit.NbSectors = 4; /*We want to erase from Sector 8 to Sector 11*/
			pEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;

			int err = HAL_FLASHEx_Erase(&pEraseInit, &SectorError);
			if(err != HAL_OK)
			{
			  print_serial_warn("Error erasing sectors. Problem sector is %x", SectorError);
			}
			else
			{
				rt = app_bootloader_build_error(build_digest, APP_BOOTLOADER_CMD_E_INVALID, "Can not erase MCU application partition");
				break;
				print_serial_info("Success erasing APP sector!");
			}
			HAL_FLASH_Lock();

			break;
		}
		default:
		{
			print_serial_error("Unknown command arrive to bootloader");
			break;
		}
	}
	return rt;
}

int app_bootloader_init(void)
{
	app_bootloader_set_state(APP_BOOTLOADER_STATE_INIT);
	return APP_BOOTLOADER_OK;
}

int app_bootloader_start(void)
{
	uint16_t recv_length = 0;
	int err = APP_BOOTLOADER_OK;
	app_bootloader_build_res_t build_digest = {0};

	memset((void *)app_bootloader_buffer, 0, sizeof(app_bootloader_buffer));
	int rt = console_recv_data(app_bootloader_buffer, &recv_length);
	if(rt == 0 && recv_length != 0)
	{
		app_bootloader_frame_t * command_digest = NULL;
		rt = app_bootloader_command_check(app_bootloader_buffer, recv_length, &command_digest);
		if(rt == APP_BOOTLOADER_CMD_OK)
		{
			app_bootloader_process_command(command_digest, &build_digest);
		}
	}

	switch(app_bootloader_get_state())
	{
		case APP_BOOTLOADER_STATE_INIT:
		{
			err = app_bootloader_build_hello(&build_digest);
			if(err != APP_BOOTLOADER_CMD_OK)
				print_serial_error("Error building 'hello' frame");
			break;
		}
		case APP_BOOTLOADER_STATE_READY:
		{
			static bool once = false;
			if(!once)
			{
				once = true;
				print_serial_info("Bootloader application ready to use!");
			}
		}
		default:
		{
			print_serial_error("Bootloader application unknown state");
			break;
		}
	}

	if(build_digest.frame)
	{
		err = console_send_data(build_digest.frame, build_digest.frame_size);
		if(err != 0)
			print_serial_error("Error sending through console");
	}

	if(build_digest.frame)
		free((void *)build_digest.frame);

	return rt;
}

