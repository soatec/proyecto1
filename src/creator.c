#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include "buffer.h"
#include "utils.h"


int shared_system_init(char* buffer_name, unsigned int buffer_size) {
    int ret;
    system_sh_state_t *system_state = NULL;
    circular_buffer_t* cbuffer;
    unsigned int wait_time_s = 1;
    unsigned int waited_time_s;
    bool system_alive;
    unsigned int producer_count;
    unsigned int consumer_count;

    // Set sbuffer to shared memory
    cbuffer = shm_cbuffer_set(buffer_name, buffer_size);
    if (!cbuffer) {
        ret = EXIT_FAILURE;
        return ret;
    }

    // Set system shared state structure to shared memory
    system_state = shm_system_state_set(buffer_name);
    if (!system_state){
        ret = EXIT_FAILURE;
        return ret;
    }

    system_state->buffer_size = buffer_size;
    system_state->keep_alive = true;
    system_state->producer_count = 0;
    system_state->consumer_count = 0;
    system_alive = system_state->keep_alive;
    producer_count = system_state->producer_count;
    consumer_count = system_state->consumer_count;
    system_state->cbuffer_address = cbuffer;

    // Initialize cbuffer empty space semaphore to buffer size
    // Semaphore, Processes != 0, Value
    ret = sem_init(&system_state->sem_cbuffer_empty, 1, buffer_size);
    if (ret) {
        fprintf(stderr, "Failed to init cbuffer empty semaphore\n");
        return ret;
    }

    // Initialize cbuffer message available semaphore to zero
    ret = sem_init(&system_state->sem_cbuffer_message, 1, 0);
    if (ret) {
        fprintf(stderr, "Failed to init cbuffer message semaphore\n");
        return ret;
    }

    // Initialize cbuffer producers counter mutex (semaphore value to one)
    ret = sem_init(&system_state->mut_producer_count, 1, 1);
    if (ret) {
        fprintf(stderr, "Failed to init producers counter mutex\n");
        return ret;
    }

    // Initialize consumers counter mutex (semaphore value to one)
    ret = sem_init(&system_state->mut_consumer_count, 1, 1);
    if (ret) {
        fprintf(stderr, "Failed to init consumer counter mutex\n");
        return ret;
    }

    // Initialize cbuffer write mutex (semaphore value to one)
    ret = sem_init(&system_state->mut_cbuffer_write, 1, 1);
    if (ret) {
        fprintf(stderr, "Failed to init cbuffer write mutex\n");
        return ret;
    }

    // Initialize cbuffer read mutex (semaphore value to one)
    ret = sem_init(&system_state->mut_cbuffer_read, 1, 1);
    if (ret) {
        fprintf(stderr, "Failed to init cbuffer read mutex\n");
        return ret;
    }

    while(system_alive || producer_count != 0 || consumer_count != 0) {
        // Creator waits for a random exponential time according the given mean
        fprintf(stdout,
                "\nCreator with PID %u has been created %u seconds ago - buffer name: %s\n",
                getpid(),
                ++waited_time_s,
                buffer_name);
        sleep(wait_time_s);

        system_alive = system_state->keep_alive;

        // Lock producer count mutex
        ret = sem_wait(&system_state->mut_producer_count);
        if (ret) {
            fprintf(stdout,
                    "\nCreator PID: %u failed to lock producer count: %s\n",
                    getpid(),
                    buffer_name);
        }

        producer_count = system_state->producer_count;

        // Unlock producer count mutex
        ret = sem_post(&system_state->mut_producer_count);
        if (ret) {
            fprintf(stdout,
                    "\nCreator PID: %u failed to unlock producer count: %s\n",
                    getpid(),
                    buffer_name);
            return ret;
        }

        // Lock consumer count mutex
        ret = sem_wait(&system_state->mut_consumer_count);
        if (ret) {
            fprintf(stdout,
                    "\nCreator PID: %u failed to lock consumer count: %s\n",
                    getpid(),
                    buffer_name);
        }

        consumer_count = system_state->consumer_count;

        // Unlock consumer count mutex
        ret = sem_post(&system_state->mut_consumer_count);
        if (ret) {
            fprintf(stdout,
                    "\nCreator PID: %u failed to unlock consumer count: %s\n",
                    getpid(),
                    buffer_name);
            return ret;
        }
    }

    return ret;
}
