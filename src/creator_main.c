#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "creator.h"

int main(int argc, char *argv[]) {
    int opt;
    int buffer_size = -1;
    char * buffer_name = NULL;

    // Get buffer name and exponential mean from console arguments
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
        fprintf(stderr, "-m buffer_size is a mandatory argument\n");
        exit(EXIT_FAILURE);
    } else if (buffer_size < 0) {
        fprintf(stderr, "-m buffer_size must be positive\n");
        exit(EXIT_FAILURE);
    }

    shared_system_init(buffer_name, buffer_size);
    exit(EXIT_SUCCESS);
}
