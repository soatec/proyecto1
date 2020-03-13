#ifndef PROYECTO1_CONSUMER_H
#define PROYECTO1_CONSUMER_H

#include "buffer.h"
#include "utils.h"

// Structs

/**
 * Consumer structure
 */
typedef struct consumer_t
{
    /** Process ID **/
    unsigned int process_id;

    /** Process ID % 5 **/
    unsigned int process_id_mod_5;

    /** Buffer file name **/
    char *buffer_name;

    /** Wait time exponential mean in seconds **/
    unsigned int exp_mean_wait_s;

    /** Cosumed message counter **/
    unsigned int message_count;

    /** Waited time in seconds **/
    unsigned int waited_time_s;

    /** Blocked time by cbuffer empty space semaphore in seconds */
    unsigned int blocked_time_by_empty_sem_s;

    /** Blocked time by cbuffer write mutex in seconds */
    unsigned int blocked_time_by_wr_mut_s;

    /** System shared state **/
    system_sh_state_t *sys_state;

    /** Shared circular buffer */
    circular_buffer_t *cbuffer;
} consumer_t;

/**
 * Initialize new producer
 *
 * @param consumer
 * @param buffer_name
 * @param mean_s
 * @return
 */
int new_consumer(consumer_t *consumer, char *buffer_name, int mean_s);

/**
 * Run consumer
 *
 * @param consumer
 * @return
 */
int run_consumer(consumer_t *consumer);

#endif //PROYECTO1_CONSUMER_H