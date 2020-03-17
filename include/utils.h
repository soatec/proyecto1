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

    /** Flag to keep producers and consumers running */
    bool keep_alive;

    /** Producers counter */
    unsigned int producer_count;

    /** Mutex to protect producers counter */
    sem_t mut_producer_count;

    /** Consumers counter */
    unsigned int consumer_count;

    /** Mutex to protect consumers counter */
    sem_t mut_consumer_count;

    /** Finalizer counter */
    unsigned int finalizer_count;

    /** Semaphore to indicate empty space in cbuffer */
    sem_t sem_cbuffer_empty;

    /** Semaphore to indicate available message in cbuffer */
    sem_t sem_cbuffer_message;

    /** Mutex to protect message write into cbuffer */
    sem_t mut_cbuffer_write;

    /** Mutex to protect message read from cbuffer */
    sem_t mut_cbuffer_read;

    /** Mutex to protect creator execution */
    sem_t mut_creator_running;

    /** cbuffer address */
    circular_buffer_t* cbuffer_address;
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
 * Returns NULL on error
 */
system_sh_state_t *shm_system_state_set(char *buffer_name);

/**
 * Get shared memory system state
 * Caller is responsive for unmapping the pointer and closing the file
 * Returns NULL on error
 */
system_sh_state_t *shm_system_state_get(char *buffer_name);

/**
 * Set shared circular buffer
 * Returns NULL on error
 */
circular_buffer_t *shm_cbuffer_set(char *buffer_name, unsigned int size);

/**
 * Get system shared state
 * Caller is responsive for unmapping the pointer and closing the file
 * Returns NULL on error
 */
circular_buffer_t *shm_cbuffer_get(char *buffer_name, unsigned int size,
                                   circular_buffer_t *cbuffer_address);

/**
 * Unmap system state shared memory pointer and close file
 * Returns 0 on success, otherwise an error code
 */
int sys_state_unmap_close(system_sh_state_t* sys_state, char* buffer_name);

/**
 * Destroy system shared state semaphores
 *  Returns 0 on success, otherwise an error code
 */
int sys_state_destroy_semaphores(system_sh_state_t* sys_state);

/**
 * Unmap cbuffer shared memory pointer and close file descriptor
 * Returns 0 on success, otherwise an error code
 */
int cbuffer_unmap_close(circular_buffer_t* cbuffer, char* buffer_name);

/**
 * Format accumulated time into seconds and milliseconds
 */
struct timeval format_accumulated_time(struct timeval time);

/**
 * Get time interval in seconds and microseconds
 */
struct timeval get_time_interval(struct timeval start_time, struct timeval end_time);

#endif //PROYECTO1_UTILS_H
