#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "creator.h"
#include "buffer.h"
#include "utils.h"

int main(int argc, char *argv[]) {
    int status;
    int opt;
    int buffer_size = -1;
    char * buffer_name = NULL;

    // Get buffer name and buffer size from console arguments
    while ((opt = getopt(argc, argv, "b:s:")) != -1) {
        switch (opt) {
            case 'b':
                buffer_name = optarg;
                break;

            case 's':
                buffer_size = atoi(optarg);
                break;

            default:
                fprintf(stderr, "Usage: %s -b buffer_name -s buffer_size\n",
                        argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (buffer_name == NULL) {
        fprintf(stderr, "-b buffer_name is a mandatory argument\n");
        exit(EXIT_FAILURE);
    }

    if (buffer_size == -1) {
        fprintf(stderr, "-s buffer_size is a mandatory argument\n");
        exit(EXIT_FAILURE);
    } else if (buffer_size < 0) {
        fprintf(stderr, "-s buffer_size must be positive\n");
        exit(EXIT_FAILURE);
    }
    system_sh_state_t system_state;
    circular_buffer_t cbuffer;

    status = new_creator(buffer_name, buffer_size, &system_state, &cbuffer);
    if (status){
        fprintf(stderr, "Error while creating buffer and other variables\n");
        return status;
    }

    status = run_creator(&system_state, buffer_name);
    if (status){
        fprintf(stderr, "Error while running creator\n");
        return status;
    }

    exit(EXIT_SUCCESS);
}
