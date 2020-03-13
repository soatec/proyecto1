#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "consumer.h"

int main(int argc, char *argv[]) {
    int status;
    int opt;
    char *buffer_name = NULL;
    int mean_s = -1;

    while ((opt = getopt(argc, argv, "b:m:")) != -1) {
        switch (opt) {
            case 'b':
                buffer_name = optarg;
                break;
            case 'm':
                mean_s = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s -b buffer_name -m exponential_mean_s\n", argv[0]);
                return EXIT_FAILURE;
        }
    }

    if (buffer_name == NULL) {
        fprintf(stderr, "-b buffer_name is a mandatory argument\n");
        return EXIT_FAILURE;
    }

    if (mean_s == -1) {
        fprintf(stderr, "-m exponential_mean_s is a mandatory argument\n");
        return EXIT_FAILURE;
    } else if (mean_s < 0) {
        fprintf(stderr, "-m exponential_mean_s must be positive\n");
        return EXIT_FAILURE;
    }

    consumer_t consumer;
    status = new_consumer(&consumer, buffer_name, mean_s);
    if (status){
        fprintf(stderr, "Error while creating consumer\n");
        return status;
    }

    status = run_consumer(&consumer);
    if (status){
        fprintf(stderr, "Error while running consumer\n");
        return status;
    }

    return EXIT_SUCCESS;
}


