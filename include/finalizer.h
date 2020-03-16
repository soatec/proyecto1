#ifndef PROYECTO1_FINALIZER_H
#define PROYECTO1_FINALIZER_H

#include <sys/time.h>
#include "utils.h"

// Structs

/**
 * Producer structure
 */
typedef struct finalizer_t
{
    /** Process ID */
    unsigned int process_id;

    /** Buffer file name */
    char *buffer_name;

    /** Produced messages counter */
    unsigned int message_count;

    /** Blocked time by cbuffer empty space semaphore in seconds */
    struct timeval blocked_time_by_empty_sem_s;

    /** Blocked time by cbuffer write mutex in seconds */
    struct timeval blocked_time_by_wr_mut_s;

    /** Waiting time for producers in seconds */
    struct timeval waiting_producers;

    /** Waiting time for consumers in seconds */
    struct timeval waiting_consumers;

    /** Whole time for finalizer to run in seconds */
    struct timeval time_elapsed;

    /** System shared state **/
    system_sh_state_t *sys_state;

    /** Shared circular buffer */
    circular_buffer_t *cbuffer;
} finalizer_t;

int new_finalizer(finalizer_t *finalizer, char* buffer_name);

int run_finalizer(finalizer_t *finalizer);

#endif //PROYECTO1_FINALIZER_H