#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

#include "app_bootloader_command.h"
#include "app_bootloader_host.h"

#define BOOTLOADER_HOST_BUFFER_SIZE (4096)

#include "API_log.h"
#define tag "app_bootloader_host.c"
#define print_serial_info(format, ...) LOG_LEVEL(LOG_INFO, tag, format, ##__VA_ARGS__)
#define print_serial_warn(format, ...) LOG_LEVEL(LOG_WARN, tag, format, ##__VA_ARGS__)
#define print_serial_error(format, ...) LOG_LEVEL(LOG_ERROR, tag, format, ##__VA_ARGS__)
#define print_serial_hex(data, data_size) LOG_HEXDUMP(tag, data, data_size, LOG_WARN)

typedef struct
{
	uint32_t actual_size;
	uint32_t total_size;
	uint32_t total_block_nbr;
	uint32_t actual_block_nbr;
	uint32_t block_size;
	app_bootloder_dl_type dl_type;
	uint8_t partition_nbr;
}app_bootloader_dl_t;

typedef enum
{
	APP_BOOTLOADER_STATE_DISABLE = -1,
	APP_BOOTLOADER_STATE_INIT,
	APP_BOOTLOADER_STATE_READY,
    APP_BOOTLOADER_STATE_DL_FINISH,
}app_bootloader_state_t;

typedef struct
{
	app_bootloader_state_t state;
	app_bootloader_dl_t dl_status;
}app_bootloader_t;

static volatile app_bootloader_t app_bootloader = {0};

static FILE * file = NULL;
static int port;

static uint8_t buffer[BOOTLOADER_HOST_BUFFER_SIZE];

static void log_output_function(uint8_t * data, uint16_t data_size)
{
    const char * format = (const char *) data;
    printf("%s", (char *) format);
}

static void bootloader_close_files(void)
{
    fclose(file);
    close(port);
}

static int bootloader_port_recv(uint8_t * buffer, uint32_t buffer_size)
{
    int recv = read(port, buffer, buffer_size);
}

static int bootloader_port_send(uint8_t * data, uint32_t data_size)
{
    return write(port, data, data_size);
}

static int bootloader_open_port(const char * name)
{
    port = open(name, O_RDWR | O_NOCTTY | O_SYNC | O_NONBLOCK);
    if(port < 0)
        return -1;

    struct termios tty;

    if (tcgetattr(port, &tty) != 0) {
        print_serial_error("Failed to get serial port attributes");
        return -1;
    }

    // Set baud rate to 115200
    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);

    // Configure 8N1 (8 data bits, no parity, 1 stop bit)
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8 data bits
    tty.c_cflag &= ~PARENB;                      // No parity bit
    tty.c_cflag &= ~CSTOPB;                      // 1 stop bit

    // No hardware flow control
    tty.c_cflag &= ~CRTSCTS;

    // Enable the receiver and set local mode
    tty.c_cflag |= (CLOCAL | CREAD);

    // Raw input/output mode
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_oflag &= ~OPOST;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);

    // Set the new attributes
    if(tcsetattr(port, TCSANOW, &tty) != 0) {
        perror("Failed to set serial port attributes");
        return -1;
    }

    return 0;
}

static int bootloader_open_file(const char * name)
{
    file = fopen(name, "rb");
    if(file != NULL)
        return 0;
    else
        return -1;
}

static uint32_t bootloader_get_file_size(void)
{
    fseek(file, 0, SEEK_END);
    uint32_t size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return size;
}

static int bootloader_read_file(uint8_t * buffer, uint32_t buffer_size)
{
    return fread(buffer, sizeof(*buffer), buffer_size, file);
}

static int app_bootloader_process_command(bool download, bool boot, app_bootloader_frame_t * command_digest, app_bootloader_build_res_t * build_digest)
{
    int rt = APP_BOOTLOADER_CMD_OK;
    switch((app_bootloader_command) command_digest->command)
    {
        case APP_BOOTLOADER_CMD_HELLO:
        {
            app_bootloader.state = APP_BOOTLOADER_STATE_READY;
            if(boot)
            {
                rt = app_bootloader_build_boot_app(build_digest, app_bootloader.dl_status.partition_nbr);
            }
            else if(download)
            {
                rt = app_bootloader_build_dl_req(build_digest, app_bootloader.dl_status.partition_nbr, app_bootloader.dl_status.total_size);
            }
            break;
        }
        case APP_BOOTLOADER_CMD_DOWNLOAD_PARAM_REQ:
        {
            app_bootloader_cmd_dl_param_req * dl_param_req =  (app_bootloader_cmd_dl_param_req *) command_digest->data;
            app_bootloader.dl_status.dl_type = dl_param_req->type;
            app_bootloader.dl_status.block_size = dl_param_req->block_size;
            app_bootloader.dl_status.actual_block_nbr = 0;
            app_bootloader.dl_status.actual_size = 0;
            app_bootloader.dl_status.total_block_nbr = (uint32_t) (app_bootloader.dl_status.total_size/app_bootloader.dl_status.block_size);

            rt = app_bootloader_build_dl_param_res(build_digest, dl_param_req->type, app_bootloader.dl_status.total_block_nbr, dl_param_req->block_size);
            break;
        }
        case APP_BOOTLOADER_CMD_DOWNLOAD_BLOCK_REQ:
        {
            app_bootloader_cmd_dl_block_req * dl_block_req = (app_bootloader_cmd_dl_block_req *) command_digest->data;
            if(app_bootloader.dl_status.actual_block_nbr != dl_block_req->block_nbr && app_bootloader.dl_status.total_block_nbr != app_bootloader.dl_status.actual_block_nbr)
            {
                print_serial_error("Not the correct block number");
                rt = APP_BOOTLOADER_CMD_E_INVALID;
                break;
            }

            uint32_t to_send = app_bootloader.dl_status.block_size;
            if(app_bootloader.dl_status.total_size < (app_bootloader.dl_status.actual_size + to_send))
                to_send = app_bootloader.dl_status.total_size - app_bootloader.dl_status.actual_size;

            uint8_t * buffer = calloc(to_send, sizeof(*buffer));

            int res = bootloader_read_file(buffer, to_send);
            if(res <= 0)
            {
                free(buffer);
                print_serial_error("Can not read file");
                rt = APP_BOOTLOADER_CMD_E_FAIL;
                break;
            }
            rt = app_bootloader_build_dl_block_res(build_digest, app_bootloader.dl_status.actual_block_nbr, res, buffer);
            app_bootloader.dl_status.actual_block_nbr++;

            free(buffer);
            break;
        }
        case APP_BOOTLOADER_CMD_DOWNLOAD_END:
        {
            app_bootloader.state = APP_BOOTLOADER_STATE_DL_FINISH;
            print_serial_info("Download complete!");
            break;
        }
        case APP_BOOTLOADER_CMD_ERROR:
        {
            app_bootloader_cmd_err * cmd_err = (app_bootloader_cmd_err *) command_digest->data;
            print_serial_error("Received error. Code %d, message [%s]", cmd_err->error, cmd_err->error_msg);
        }
        default:
        {
            print_serial_error("Unknown command to process");
            break;
        }
    }
    return rt;
}

int main(int argc, char ** argv)
{
    /* Set logs */
    log_set_transmit_function(log_output_function);
    print_serial_info("Start bootloader application for host...");

    char *fvalue = NULL;
    char *pvalue = NULL;
    int  partition_nbr = -1;
    bool dflag = false;
    bool iflag = false;
    bool bflag = false;
    int  index;
    int  c;

    opterr = 0;
    while ((c = getopt (argc, argv, "bidp:n:f:")) != -1){
        switch (c)
        {     
            case 'b':
            {
                bflag = true;
                break;
            }
            case 'f':
                if(!iflag || bflag)
                    fvalue = optarg;
                break;
            case 'n':
            {
                if(!iflag || bflag)
                {
                    if(optarg)
                    {
                        if(isdigit(optarg[0]))
                            partition_nbr = atoi(optarg);
                        else
                            print_serial_error("-p argument must be a digit");
                    }
                }
                break;
            }
            case 'p':
            {
                if(!iflag || bflag)
                    pvalue = optarg;
                break;
            }
            case 'd':
                if(!iflag)
                    dflag = true;
                break;
            case 'i':
                if(!dflag)
                    iflag = true;
                break;
            case '?':
                if (optopt == 'p' && !iflag)
                    print_serial_error("Option -%c requires an serial port.", optopt);
                else if(optopt == 'f' && !iflag)
                    print_serial_error("Option -%c requires a file program.", optopt);
                else if(optopt == 'n' && !iflag)
                    print_serial_error("Option -%c requires a [0-9] partition number.", optopt);
                else if (isprint (optopt))
                    print_serial_error("Unknown option `-%c'.", optopt);
                else
                    print_serial_error("Unknown option character `\\x%x'.",optopt);
                return 1;
            default:
                exit(1);
        }
    }

    if(iflag)
    {
        /* Request partition info */
    }
    else if((bflag || dflag) && fvalue != NULL && partition_nbr != -1 && pvalue != NULL)
    {
        /* Start download or boot */
        int err = bootloader_open_file(fvalue);
        if(err != 0)
        {
            print_serial_error("Can not load program file");
            goto finish;
        }
        
        err = bootloader_open_port(pvalue);
        if(err != 0)
        {
            print_serial_error("Can not open port");
            goto finish;
        }

        app_bootloader.dl_status.total_size = bootloader_get_file_size();
        app_bootloader.dl_status.partition_nbr = (uint8_t)partition_nbr;
        print_serial_info("All set to download %d bytes file", app_bootloader.dl_status.total_size);

        print_serial_info("Waiting for client response");

        while(true)
        {
            app_bootloader_build_res_t build_digest = {0};
            if(app_bootloader.state == APP_BOOTLOADER_STATE_INIT)
            {
                int rt = app_bootloader_build_host_hello(&build_digest);
            }

            memset(buffer, 0, sizeof(buffer));
            int recv = bootloader_port_recv(buffer, sizeof(buffer));
            if(recv > 0)
            {
                print_serial_warn("Received from client");
                print_serial_hex(buffer, recv);
                app_bootloader_frame_t * command_digest = NULL;
                int rt = app_bootloader_command_check(buffer, recv, &command_digest);
                print_serial_warn("Check command returned %d", rt);
                if(rt == APP_BOOTLOADER_CMD_OK)
                {
                    rt = app_bootloader_process_command(dflag, bflag, command_digest, &build_digest);
                    print_serial_warn("Process command returned %d", rt);
                }
            }

            if(build_digest.frame != NULL)
            {
                int err = bootloader_port_send(build_digest.frame, build_digest.frame_size);
                print_serial_warn("Send port returned %d", err);
                usleep(500000);
                free(build_digest.frame);
            }

            if(app_bootloader.state == APP_BOOTLOADER_STATE_DL_FINISH)
                break;
        }
    }
    else
    {
        print_serial_error("Can not execute application. Missing parameters");
    }

finish:
    bootloader_close_files();
    return 0;
}