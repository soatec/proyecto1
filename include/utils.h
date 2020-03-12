#ifndef PROYECTO1_UTILS_H
#define PROYECTO1_UTILS_H

#include <semaphore.h>
#include <stdlib.h>
#include "buffer.h"

/**
 * System shared state structure
 */
typedef struct system_sh_state {
    /** Flag to keep producers and consumers running */
    unsigned int buffer_size;

    /** Mutex to protect buffer size */
    sem_t mut_buffer_size;

    /** Flag to keep producers and consumers running */
    bool keep_alive;

    /** Mutex to protect keep alive */
    sem_t mut_keep_alive;

    /** Producers counter */
    unsigned int producer_count;

    /** Mutex to protect producers counter */
    sem_t mut_producer_count;

    /** Consumers counter */
    unsigned int consumer_count;

    /** Mutex to protect consumers counter */
    sem_t mut_consumer_count;

    /** Semaphore to indicate empty space in cbuffer */
    sem_t sem_cbuffer_empty;

    /** Semaphore to indicate available message in cbuffer */
    sem_t sem_cbuffer_message;

    /** Mutex to protect message write into cbuffer */
    sem_t mut_cbuffer_write;

    /** Mutex to protect message read from cbuffer */
    sem_t mut_cbuffer_read;
} system_sh_state_t;

/**
 * Returns an exponential random number from a given mean
 */
unsigned int exponential_random_get(unsigned int mean);

/**
 * Prints a message
 */
void message_print(message_t message);

/**
 * Set shared memory system state
 */
system_sh_state_t *shm_system_state_set(char *buffer_name);

/**
 * Get shared memory system state
 * Caller is responsive for unmapping the pointer and closing the file
 */
system_sh_state_t *shm_system_state_get(char *buffer_name);

/**
 * Set shared circular buffer
 */
circular_buffer_t *shm_cbuffer_set(char *buffer_name, unsigned int size);

/**
 * Get system shared state
 * Caller is responsive for unmapping the pointer and closing the file
 */
circular_buffer_t *shm_cbuffer_get(char *buffer_name, unsigned int size);

/**
 * Unmap system state shared memory pointer and close file
 * Returns 0 on success, otherwise an error code
 */
int sys_state_unmap_close(system_sh_state_t* sys_state, char* buffer_name);

/**
 * Unmap cbuffer shared memory pointer and close file descriptor
 * Returns 0 on success, otherwise an error code
 */
int cbuffer_unmap_close(circular_buffer_t* cbuffer, char* buffer_name);

#endif //PROYECTO1_UTILS_H
