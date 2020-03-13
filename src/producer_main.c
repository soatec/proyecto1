#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "producer.h"

int main(int argc, char *argv[]) {
    int opt;
    int status;
    int mean_s = -1;
    char * buffer_name = NULL;
    producer_t producer;

    // Get buffer name and exponential mean from console arguments
    while ((opt = getopt(argc, argv, "b:m:")) != -1) {
        switch (opt) {
            case 'b':
                buffer_name = optarg;
                break;

            case 'm':
                mean_s = atoi(optarg);
                break;

            default:
                fprintf(stderr, "Usage: %s -b buffer_name -m exponential_mean_s\n",
                        argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (buffer_name == NULL) {
        fprintf(stderr, "-b buffer_name is a mandatory argument\n");
        exit(EXIT_FAILURE);
    }

    if (mean_s == -1) {
        fprintf(stderr, "-m exponential_mean_s is a mandatory argument\n");
        exit(EXIT_FAILURE);
    } else if (mean_s < 0) {
        fprintf(stderr, "-m exponential_mean_s must be positive\n");
        exit(EXIT_FAILURE);
    }

    status = new_producer(&producer, buffer_name, mean_s);
    if (status){
        fprintf(stderr, "Error while creating producer\n");
        return status;
    }

    status = run_producer(&producer);
    if (status){
        fprintf(stderr, "Error while running producer\n");
        return status;
    }

    exit(EXIT_SUCCESS);
}