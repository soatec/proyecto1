#ifndef PROYECTO1_PRODUCER_H
#define PROYECTO1_PRODUCER_H

#include "buffer.h"
#include "utils.h"

// Structs

/**
 * Producer structure
 */
typedef struct producer_t
{
    /** Process ID */
    unsigned int process_id;

    /** Buffer file name */
    char *buffer_name;

    /** Wait time exponential mean in seconds */
    unsigned int exp_mean_wait_s;

    /** Produced messages counter */
    unsigned int message_count;

    /** Waited time in seconds */
    unsigned int waited_time_s;

    /** Blocked time by semaphores in seconds */
    unsigned int blocked_time_s;

    /** System shared state **/
    system_sh_state_t *sys_state;

    /** Shared circular buffer */
    circular_buffer_t *cbuffer;
} producer_t;

/**
 * Initialize new producer
 *
 * @param producer
 * @param buffer_name
 * @param exp_mean
 * @return
 */
int new_producer(producer_t *producer, char *buffer_name, unsigned int exp_mean);

/**
 * Run producer
 *
 * @param producer
 * @return
 */
int run_producer(producer_t *producer);

#endif //PROYECTO1_PRODUCER_H
