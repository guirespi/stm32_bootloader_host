#include <stdio.h>
#include <windows.h>

#include <ezm_api_console.h>
#include <ezm_pp_ep_drv.h>
#include <ezm_pp_drv.h>
#include <ezm_pp_main.h>

#include <ezm_log_gen.h>
DEFINE_TAG();


#define PRODUCTION_PORT_OPENED (1<<0)
#define PRODUCTION_IMG_UPLOADED (1<<1)

#define CONSOLE_OUTPUT(format, ...) \
    printf(format, ##__VA_ARGS__)

uint32_t flag = 0;
uint8_t * file_content = 0;
uint32_t file_size = 0;

typedef int (*cmd_fn_t)(int argc, char **argv);

typedef struct {
    const char *name;
    const char *help;
    cmd_fn_t fn;
} app_prod_command_t;

static int cmd_help(int argc, char **argv)
{
    (void)argc; (void)argv;
    CONSOLE_OUTPUT("help    - Shows this help\n");
    CONSOLE_OUTPUT("open    - Opens the serial port\n");
    CONSOLE_OUTPUT("ident   - Identifies the device\n");
    CONSOLE_OUTPUT("info    - Request image information from the device\n");
    CONSOLE_OUTPUT("upload  - Upload an image\n");
    CONSOLE_OUTPUT("load    - Load an image to the device\n");
    CONSOLE_OUTPUT("boot    - Boot the device with selected image\n");
    CONSOLE_OUTPUT("exit    - Goodbye!\n");
    return 0;
}

static int cmd_exit(int argc, char **argv)
{
    (void)argc; (void)argv;
    return 1; // señal de salida
}

static int cmd_ident(int argc, char **argv)
{
    (void)argc; (void)argv;
    ezm_pp_drv_set_msg(0, EZM_PP_ACTION_IDENT_VERSION, EZM_PP_DRV_SERIAL, 0);
    ezm_pp_action_t action = EZM_PP_NO_ACTION;
    while(1) {
        action = ezm_pp_main();
        if(action == EZM_PP_NO_ACTION)
            break;
    }
    return 0;
}

static int cmd_info(int argc, char **argv)
{
    if(argc < 2) {
        CONSOLE_OUTPUT("Usage: info [img_index]\n");
        return 0;
    }
    if(isdigit(argv[1][0])) { 
        uint8_t img_index = (uint8_t)strtol(argv[1], NULL, 10);
        ezm_pp_drv_set_msg(0, EZM_PP_ACTION_IDENT_IMG_INFO, EZM_PP_DRV_SERIAL, img_index);
            ezm_pp_action_t action = EZM_PP_NO_ACTION;
        while(1) {
            action = ezm_pp_main();
            if(action == EZM_PP_NO_ACTION)
                break;
        }
    } else {
        CONSOLE_OUTPUT("Invalid image index\n");
    }
    return 0;
}

static int cmd_upload(int argc, char **argv)
{
    if(argc < 2) {
        CONSOLE_OUTPUT("Usage: upload [image_path]\n");
        return 0;
    }

    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        flag &= ~PRODUCTION_IMG_UPLOADED;
        CONSOLE_OUTPUT("Failed to open file: %s\n", argv[1]);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    file_content = (uint8_t *)realloc(file_content, file_size);
    if (!file_content) {
        flag &= ~PRODUCTION_IMG_UPLOADED;
        CONSOLE_OUTPUT("Memory allocation failed\n");
        fclose(file);
        return 0;
    }

    size_t read_size = fread(file_content, 1, file_size, file);
    fclose(file);

    if(read_size != file_size) {
        flag &= ~PRODUCTION_IMG_UPLOADED;
        CONSOLE_OUTPUT("Failed to read file: %s\n", argv[1]);
        free(file_content);
        file_content = NULL;
        file_size = 0;
        return 0;
    }

    CONSOLE_OUTPUT("Image uploaded\n");
    flag |= PRODUCTION_IMG_UPLOADED;
    return 0;
}

static int cmd_load(int argc, char **argv)
{
    if(flag & PRODUCTION_IMG_UPLOADED == 0) {
        CONSOLE_OUTPUT("No image uploaded. Use 'upload' command first.\n");
        return 0;
    }

    if(argc < 2) {
        CONSOLE_OUTPUT("Usage: load [img_index]\n");
        return 0;
    }

    ezm_pp_action_t action = EZM_PP_NO_ACTION;
    if(isdigit(argv[1][0])) { 
        ezm_pp_ep_drv_prepare_host_download(file_content, file_size, EZM_PP_MAX_BLOCK_SIZE);
        uint8_t img_index = (uint8_t)strtol(argv[1], NULL, 10);
        ezm_pp_drv_set_msg(0, EZM_PP_ACTION_PREPARE_DOWNLOAD, EZM_PP_DRV_SERIAL, img_index);
        while(1) {
            action = ezm_pp_main();
            if(action == EZM_PP_NO_ACTION || 
                action == EZM_PP_ACTION_HOST_ERROR_DOWNLOAD ||
                action == EZM_PP_ACTION_HOST_END_DOWNLOAD)
                break;
        }
    } else {
        CONSOLE_OUTPUT("Invalid image index\n");
    }

    if(action == EZM_PP_ACTION_HOST_ERROR_DOWNLOAD) {
        CONSOLE_OUTPUT("Error while downloading!\n");
    } else if (action == EZM_PP_ACTION_HOST_END_DOWNLOAD) {
        CONSOLE_OUTPUT("Download succesful!\n");
    }

    return 0;
}

app_prod_command_t commands[] = {
    { "help", "Shows all commands", cmd_help },
    { "exit", "Goodbye!", cmd_exit },
    { "ident", "Identify the device", cmd_ident },
    { "info", "Request image information from the device", cmd_info },
    { "upload", "Upload an image", cmd_upload },
    { "load", "Load an image to the device", cmd_load },
};

static int tokenize(char *line, char **argv, int max)
{
    int argc = 0;
    char *tok = strtok(line, " ");

    while (tok && argc < max) {
        argv[argc++] = tok;
        tok = strtok(NULL, " ");
    }
    return argc;
}

static int dispatch(int argc, char **argv)
{
    for (size_t i = 0; i < sizeof(commands)/sizeof(commands[0]); i++) {
        if (strcmp(argv[0], commands[i].name) == 0) {
            return commands[i].fn(argc, argv);
        }
    }
    CONSOLE_OUTPUT("Unknown command\n");
    return 0;
}

int main(void)
{
    ezm_console_init("COM7");
    CONSOLE_OUTPUT("Welcome to production application!\n");
    flag |= PRODUCTION_PORT_OPENED;

    while(1) {
        CONSOLE_OUTPUT(">");
        uint8_t input_buffer[128] = {0};

        if (!fgets(input_buffer, sizeof(input_buffer), stdin))
            break;
        
        input_buffer[strcspn(input_buffer, "\r\n")] = 0;

        char *argv[10];
        int argc = tokenize(input_buffer, argv, 10);

        if (argc == 0)
            continue;

        if (dispatch(argc, argv))
            break;
    }
    return 0;
}


