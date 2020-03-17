#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include "buffer.h"
#include "utils.h"

// Creator wait time in seconds
#define CREATOR_WAIT_TIME_S 1

system_sh_state_t *new_creator(char* buffer_name, unsigned int buffer_size)
{
    int ret;
    system_sh_state_t *system_state = NULL;
    circular_buffer_t *cbuffer;

    // Try to get system shared state
    fprintf(stdout, "\nTest that only one creator is running, an error print is"
            " expected\n");
    system_state = shm_system_state_get(buffer_name);
    if (system_state){
        fprintf(stderr, "\nFailed to init system shared state, probably another"
                " creator is running\n");
        return NULL;
    }

    // Set sbuffer to shared memory
    cbuffer = shm_cbuffer_set(buffer_name, buffer_size);
    if (!cbuffer) {
        return system_state;
    }

    // Set system shared state structure to shared memory
    system_state = shm_system_state_set(buffer_name);
    if (!system_state) {
        cbuffer_unmap_close(cbuffer, buffer_name);
        return system_state;
    }

    // Initialize creator running mutex in locked state (semaphore value to zero)
    ret = sem_init(&system_state->mut_creator_running, 1, 0);
    if (ret) {
        fprintf(stderr, "Failed to init creator running mutex\n");
        cbuffer_unmap_close(cbuffer, buffer_name);
        sys_state_unmap_close(system_state, buffer_name);
        system_state = NULL;
        return system_state;
    }

    system_state->buffer_size = buffer_size;
    system_state->keep_alive = true;
    system_state->producer_count = 0;
    system_state->consumer_count = 0;
    system_state->finalizer_count = 0;
    system_state->cbuffer_address = cbuffer;

    // Initialize cbuffer empty space semaphore to buffer size
    // Semaphore, Processes != 0, Value
    ret = sem_init(&system_state->sem_cbuffer_empty, 1, buffer_size);
    if (ret) {
        fprintf(stderr, "Failed to init cbuffer empty semaphore\n");
    }

    // Initialize cbuffer message available semaphore to zero
    ret |= sem_init(&system_state->sem_cbuffer_message, 1, 0);
    if (ret) {
        fprintf(stderr, "Failed to init cbuffer message semaphore\n");
    }

    // Initialize cbuffer producers counter mutex (semaphore value to one)
    ret |= sem_init(&system_state->mut_producer_count, 1, 1);
    if (ret) {
        fprintf(stderr, "Failed to init producers counter mutex\n");
    }

    // Initialize consumers counter mutex (semaphore value to one)
    ret |= sem_init(&system_state->mut_consumer_count, 1, 1);
    if (ret) {
        fprintf(stderr, "Failed to init consumer counter mutex\n");
    }

    // Initialize cbuffer write mutex (semaphore value to one)
    ret |= sem_init(&system_state->mut_cbuffer_write, 1, 1);
    if (ret) {
        fprintf(stderr, "Failed to init cbuffer write mutex\n");
    }

    // Initialize cbuffer read mutex (semaphore value to one)
    ret |= sem_init(&system_state->mut_cbuffer_read, 1, 1);
    if (ret) {
        fprintf(stderr, "Failed to init cbuffer read mutex\n");
        sys_state_destroy_semaphores(system_state);
        cbuffer_unmap_close(cbuffer, buffer_name);
        sys_state_unmap_close(system_state, buffer_name);
        system_state = NULL;
        return system_state;
    }

    return system_state;
}

int run_creator(system_sh_state_t *system_state, char* buffer_name) {
    int ret = EXIT_SUCCESS;
    unsigned int waited_time_s;
    bool system_alive = system_state->keep_alive;
    unsigned int producer_count = system_state->producer_count;
    unsigned int consumer_count = system_state->consumer_count;

    while(system_alive || producer_count != 0 || consumer_count != 0) {
        fprintf(stdout,
                "\nCreator PID: %u for buffer: %s has been created %u seconds ago\n",
                getpid(),
                buffer_name,
                waited_time_s);
        sleep(CREATOR_WAIT_TIME_S);

        waited_time_s += CREATOR_WAIT_TIME_S;

        system_alive = system_state->keep_alive;
        producer_count = system_state->producer_count;
        consumer_count = system_state->consumer_count;

        fprintf(stdout,
                "\nCreator PID: %u for buffer: %s {\n"
                " system_alive: %s\n"
                " producer_count: %u\n"
                " consumer_count: %u\n"
                "}\n",
                getpid(), buffer_name,
                (system_alive) ? "true" : "false",
                producer_count, consumer_count);
    }

    // Unlock creator running mutex
    ret = sem_post(&system_state->mut_creator_running);
    if (ret) {
        fprintf(stderr,
                "\nCreator PID: %u for buffer: %s failed to unlock creator runnning\n",
                getpid(),
                buffer_name);
        sys_state_destroy_semaphores(system_state);
        cbuffer_unmap_close(system_state->cbuffer_address, buffer_name);
        sys_state_unmap_close(system_state, buffer_name);
        system_state = NULL;
        return ret;
    }

    fprintf(stdout,
            "\nCreator PID: %u for buffer: %s has finalized {\n"
            " Accumulated waiting time %u\n"
            "}\n",
            getpid(),
            buffer_name,
            waited_time_s);

    return ret;
}
