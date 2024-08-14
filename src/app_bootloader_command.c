#include <stdlib.h>
#include <string.h>
#include "app_bootloader_command.h"

int static app_bootloader_command_build(app_bootloader_command command, uint8_t * data, uint32_t data_size, app_bootloader_build_res_t * build_digest)
{
	if(build_digest == NULL) return APP_BOOTLOADER_CMD_E_NULL;
	if(command  > APP_BOOTLOADER_CMD_MAX) return APP_BOOTLOADER_CMD_E_UNKNOWN;
	if(data != NULL && data_size == 0) return APP_BOOTLOADER_CMD_E_PARAM;

	app_bootloader_frame_t * cmd = NULL;
	uint16_t frame_size = sizeof(*cmd) + data_size;
	uint8_t * frame = calloc(frame_size, sizeof(*frame));
	if(frame == NULL) return APP_BOOTLOADER_CMD_E_MEM;

	cmd = (app_bootloader_frame_t *) frame;
	cmd->magic = APP_BOOTLOADER_CMD_MAGIC_BYTE;
	cmd->command = command;
	cmd->total_length = (uint16_t)data_size;

	build_digest->frame = frame;
	build_digest->frame_size = frame_size;

	memcpy(frame + sizeof(*cmd), data, data_size);
	return APP_BOOTLOADER_CMD_OK;
}

int app_bootloader_build_hello(app_bootloader_build_res_t * build_digest)
{
	return app_bootloader_command_build(APP_BOOTLOADER_CMD_HELLO, NULL, 0, build_digest);
}

int app_bootloader_build_host_hello(app_bootloader_build_res_t * build_digest)
{
	return app_bootloader_command_build(APP_BOOTLOADER_CMD_HOST_HELLO, NULL, 0, build_digest);
}

int app_bootloader_build_dl_req(app_bootloader_build_res_t * build_digest, uint8_t partition_nbr, uint32_t binary_size)
{
	app_bootloader_cmd_dl_req cmd_data = {.part_nbr = partition_nbr, .binary_size = binary_size};
	void * data = (void *) &cmd_data;
	uint32_t data_size = sizeof(cmd_data);
	return app_bootloader_command_build(APP_BOOTLOADER_CMD_DOWNLOAD_REQ, data, data_size, build_digest);
}

int app_bootloader_build_dl_param_req(app_bootloader_build_res_t * build_digest, uint8_t type, uint16_t block_size)
{
	app_bootloader_cmd_dl_param_req cmd_data = {.block_size = block_size, .type = type};
	void * data = (void *) &cmd_data;
	uint32_t data_size = sizeof(cmd_data);
	return app_bootloader_command_build(APP_BOOTLOADER_CMD_DOWNLOAD_PARAM_REQ, data, data_size, build_digest);
}

int app_bootloader_build_dl_param_res(app_bootloader_build_res_t * build_digest, uint8_t type, uint32_t total_block_nbr, uint16_t block_size)
{
	app_bootloader_cmd_dl_param_res cmd_data = {.type = type, .total_block_nbr = total_block_nbr, .block_size = block_size};
	void * data = (void *) &cmd_data;
	uint32_t data_size = sizeof(cmd_data);

	return app_bootloader_command_build(APP_BOOTLOADER_CMD_DOWNLOAD_PARAM_RES, data, data_size, build_digest);
}

int app_bootloader_build_dl_block_req(app_bootloader_build_res_t * build_digest, uint32_t block_nbr)
{
	app_bootloader_cmd_dl_block_req cmd_data = {.block_nbr = block_nbr};
	void * data = (void *) &cmd_data;
	uint32_t data_size = sizeof(cmd_data);
	return app_bootloader_command_build(APP_BOOTLOADER_CMD_DOWNLOAD_BLOCK_REQ, data, data_size, build_digest);
}

int app_bootloader_build_dl_block_res(app_bootloader_build_res_t * build_digest, uint32_t block_nbr, uint32_t data_size, uint8_t * data)
{
	app_bootloader_cmd_dl_block_res * cmd_data = NULL;
	uint32_t buffer_size = sizeof(*cmd_data) + data_size;
	void * buffer = (void *) malloc(buffer_size);
	if(buffer == NULL) return APP_BOOTLOADER_CMD_E_MEM;

	/* Pass info into structure before build */
	cmd_data = (app_bootloader_cmd_dl_block_res *) (buffer);
	cmd_data->block_nbr = block_nbr;
	cmd_data->data_size = data_size;
	memcpy(cmd_data->data, data, data_size);

	int rt = app_bootloader_command_build(APP_BOOTLOADER_CMD_DOWNLOAD_BLOCK_RES, (uint8_t *)buffer, buffer_size, build_digest);
	free(buffer);
	return rt;
}

int app_bootloader_build_dl_end(app_bootloader_build_res_t * build_digest)
{
	return app_bootloader_command_build(APP_BOOTLOADER_CMD_END, NULL, 0, build_digest);
}

int app_bootloader_build_boot_app(app_bootloader_build_res_t * build_digest, uint8_t partition_nbr)
{
	app_bootloader_cmd_boot_app cmd_data = {.partition_nbr = partition_nbr};
	void * data = (void *) &cmd_data;
	uint32_t data_size = sizeof(cmd_data);
	return app_bootloader_command_build(APP_BOOTLOADER_CMD_BOOT_APP, data, data_size, build_digest);
}

int app_bootloader_build_error(app_bootloader_build_res_t * build_digest, uint8_t error, char * message)
{
	if(build_digest == NULL) return APP_BOOTLOADER_CMD_E_NULL;
	if(message == NULL) return APP_BOOTLOADER_CMD_E_PARAM;
	app_bootloader_cmd_err * cmd_data = NULL;
	uint32_t buffer_size = sizeof(*cmd_data) + strlen(message) + 1;
	void * buffer = (void *) malloc(buffer_size);
	if(buffer == NULL) return APP_BOOTLOADER_CMD_E_MEM;

	cmd_data = (app_bootloader_cmd_err *) buffer;
	cmd_data->error = error;
	strcpy(cmd_data->error_msg, message);

	int rt = app_bootloader_command_build(APP_BOOTLOADER_CMD_ERROR, (uint8_t *)buffer, buffer_size, build_digest);
	free(buffer);
	return rt;
}

int app_bootloader_command_check(uint8_t * buffer, uint16_t buffer_size, app_bootloader_frame_t ** command_digest)
{
	if(buffer == NULL || buffer_size == 0) return APP_BOOTLOADER_CMD_E_NULL;
	/* Check the buffer size to assure we have the minimum size for an application bootloader frame */
	if(buffer_size < sizeof(**command_digest)) return APP_BOOTLOADER_CMD_E_SIZE;
	if(command_digest == NULL) return APP_BOOTLOADER_CMD_E_NULL;
	if(*command_digest != NULL) return APP_BOOTLOADER_CMD_E_PARAM;

	int res = APP_BOOTLOADER_CMD_E_INVALID;
	app_bootloader_frame_t * frame = (app_bootloader_frame_t *) buffer;

	if(frame->magic != APP_BOOTLOADER_CMD_MAGIC_BYTE) return APP_BOOTLOADER_CMD_E_INVALID;

	frame->total_length = frame->total_length;

	switch((app_bootloader_command) frame->command)
	{
		case APP_BOOTLOADER_CMD_RETRANSMIT:
		case APP_BOOTLOADER_CMD_HELLO:
		{
			res = APP_BOOTLOADER_CMD_OK;
			break;
		}
		case APP_BOOTLOADER_CMD_DOWNLOAD_PARAM_REQ:
		{
			if(frame->total_length == sizeof(app_bootloader_cmd_dl_param_req))
			{
				app_bootloader_cmd_dl_param_req * dl_param_req = (app_bootloader_cmd_dl_param_req *)frame->data;
				dl_param_req->block_size = dl_param_req->block_size;
				res = APP_BOOTLOADER_CMD_OK;
			}
			break;
		}
		case APP_BOOTLOADER_CMD_DOWNLOAD_BLOCK_REQ:
		{
			if(frame->total_length == sizeof(app_bootloader_cmd_dl_block_req))
			{
				app_bootloader_cmd_dl_block_req * dl_block_req = (app_bootloader_cmd_dl_block_req *) frame->data;
				dl_block_req->block_nbr = dl_block_req->block_nbr;
				res = APP_BOOTLOADER_CMD_OK;
			}
			break;
		}
		case APP_BOOTLOADER_CMD_END:
		{
			res = APP_BOOTLOADER_CMD_OK;
			break;
		}
		default:
		{
			res = APP_BOOTLOADER_CMD_E_UNKNOWN;
			break;
		}
	}

	if(res == APP_BOOTLOADER_CMD_OK)
		*command_digest = (app_bootloader_frame_t *) buffer;

	return res;
}
