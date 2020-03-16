#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include "finalizer.h"
#include "buffer.h"

int new_finalizer(finalizer_t *finalizer, char *buffer_name){
    finalizer->process_id = getpid();
    finalizer->buffer_name = buffer_name;
    finalizer->message_count = 0;
    finalizer->blocked_time_by_empty_sem_s = (struct timeval){0};
    finalizer->blocked_time_by_wr_mut_s = (struct timeval){0};

    finalizer->sys_state = shm_system_state_get(buffer_name);
    if (!finalizer->sys_state) return EXIT_FAILURE;

    finalizer->cbuffer = shm_cbuffer_get(buffer_name,
                                        finalizer->sys_state->buffer_size,
                                        finalizer->sys_state->cbuffer_address);
    if (!finalizer->cbuffer) return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

int run_finalizer(finalizer_t *finalizer){

    //Start finishing producers ASAP
    finalizer->sys_state->keep_alive = false;

    int ret;
    message_t message;
    struct timeval start_time, end_time, time_interval, start_p_c, end_p_c, start_all, end_all;
    unsigned int cbuffer_index;
    unsigned int producer_count, consumer_count;


    // Get current time
    ret = gettimeofday(&start_all, NULL);
    if (ret) {
        fprintf(stderr,
                "\nFinalizer PID: %u for buffer: %s failed to get current time\n",
                finalizer->process_id,
                finalizer->buffer_name);
        return ret;
    }

    // Get producers and consumers count
    producer_count = finalizer->sys_state->producer_count;
    consumer_count = finalizer->sys_state->consumer_count;

    // Get current time
    ret = gettimeofday(&start_p_c, NULL);
    if (ret) {
        fprintf(stderr,
                "\nFinalizer PID: %u for buffer: %s failed to get current time\n",
                finalizer->process_id,
                finalizer->buffer_name);
        return ret;
    }

    //Will send finalize messages until consumers are all terminated.
    while(consumer_count>0){
        // Get current time
        message.time_of_creation = time(NULL);

        // Get consumer key -1 for finalization
        message.consumer_key = -1;

        // Get current time
        ret = gettimeofday(&start_time, NULL);
        if (ret) {
            fprintf(stderr,
                    "\nFinalizer PID: %u for buffer: %s failed to get current time\n",
                    finalizer->process_id,
                    finalizer->buffer_name);
            return ret;
        }

        // Lock empty semaphore
        ret = sem_wait(&finalizer->sys_state->sem_cbuffer_empty);
        if (ret) {
            fprintf(stderr,
                    "\nFinalizer PID: %u for buffer: %s failed to lock empty space\n",
                    finalizer->process_id,
                    finalizer->buffer_name);
            return ret;
        }

        // Get current time
        ret = gettimeofday(&end_time, NULL);
        if (ret) {
            fprintf(stderr,
                    "\nFinalizer PID: %u for buffer: %s failed to get current time\n",
                    finalizer->process_id,
                    finalizer->buffer_name);
            return ret;
        }

        // Compute blocked time by cbuffer empty space semaphore
        time_interval = get_time_interval(start_time, end_time);
        finalizer->blocked_time_by_empty_sem_s.tv_sec += time_interval.tv_sec;
        finalizer->blocked_time_by_empty_sem_s.tv_usec += time_interval.tv_usec;

        // Get current time
        ret = gettimeofday(&start_time, NULL);
        if (ret) {
            fprintf(stderr,
                    "\nFinalizer PID: %u for buffer: %s failed to get current time\n",
                    finalizer->process_id,
                    finalizer->buffer_name);
            return ret;
        }

        // Lock cbuffer write mutex
        ret = sem_wait(&finalizer->sys_state->mut_cbuffer_write);
        if (ret) {
            fprintf(stderr,
                    "\nFinalizer PID: %u for buffer: %s failed to lock cbuffer write\n",
                    finalizer->process_id,
                    finalizer->buffer_name);
            return ret;
        }

        // Get current time
        ret = gettimeofday(&end_time, NULL);
        if (ret) {
            fprintf(stderr,
                    "\nFinalizer PID: %u for buffer: %s failed to get current time\n",
                    finalizer->process_id,
                    finalizer->buffer_name);
            return ret;
        }

        // Compute blocked time by cbuffer write mutex
        time_interval = get_time_interval(start_time, end_time);
        finalizer->blocked_time_by_wr_mut_s.tv_sec += time_interval.tv_sec;
        finalizer->blocked_time_by_wr_mut_s.tv_usec += time_interval.tv_usec;

        // Write message into shared buffer
        cbuffer_index = circular_buffer_put(finalizer->cbuffer, message);
        if (cbuffer_index < 0) {
            fprintf(stderr,
                    "\nFinalizer PID: %u for buffer: %s failed to write into buffer\n",
                    finalizer->process_id,
                    finalizer->buffer_name);
            ret = EXIT_FAILURE;
            return ret;
        }

        // Unlock cbuffer write mutex
        ret = sem_post(&finalizer->sys_state->mut_cbuffer_write);
        if (ret) {
            fprintf(stderr,
                    "\nFinalizer PID: %u for buffer: %s failed to unlock cbuffer write\n",
                    finalizer->process_id,
                    finalizer->buffer_name);
            return ret;
        }

        // Post message semaphore
        ret = sem_post(&finalizer->sys_state->sem_cbuffer_message);
        if (ret) {
            fprintf(stderr,
                    "\nFinalizer PID: %u for buffer: %s failed to post message semaphore\n",
                    finalizer->process_id,
                    finalizer->buffer_name);
            return ret;
        }

        // Get producers and consumers count
        producer_count = finalizer->sys_state->producer_count;
        consumer_count = finalizer->sys_state->consumer_count;


        // Increment produced messages count
        finalizer->message_count += 1;


        // Print message
        fprintf(stdout, "\nBuffer: %s was written at index %u by finalizer\n"
                " Producers counter %u, Consumers counter %u\n",
                finalizer->buffer_name, cbuffer_index, producer_count,
                consumer_count);
        message_print(message);
    }

    // Get current time
    ret = gettimeofday(&end_p_c, NULL);
    if (ret) {
        fprintf(stderr,
                "\nFinalizer PID: %u for buffer: %s failed to get current time\n",
                finalizer->process_id,
                finalizer->buffer_name);
        return ret;
    }
    
    // Compute time waiting for consumers
    time_interval = get_time_interval(start_p_c, end_p_c);
    finalizer->waiting_consumers.tv_sec = time_interval.tv_sec;
    finalizer->waiting_consumers.tv_usec = time_interval.tv_usec;

    // Get current time
    ret = gettimeofday(&start_p_c, NULL);
    if (ret) {
        fprintf(stderr,
                "\nFinalizer PID: %u for buffer: %s failed to get current time\n",
                finalizer->process_id,
                finalizer->buffer_name);
        return ret;
    }

    //Will wait for producers before unmapping
    while(producer_count > 0){
        //signal producers????
        producer_count = finalizer->sys_state->producer_count;
    }

    // Get current time
    ret = gettimeofday(&end_p_c, NULL);
    if (ret) {
        fprintf(stderr,
                "\nFinalizer PID: %u for buffer: %s failed to get current time\n",
                finalizer->process_id,
                finalizer->buffer_name);
        return ret;
    }

    // Compute time waiting for producers
    time_interval = get_time_interval(start_p_c, end_p_c);
    finalizer->waiting_producers.tv_sec = time_interval.tv_sec;
    finalizer->waiting_producers.tv_usec = time_interval.tv_usec;


    //Unmap buffer
    ret = cbuffer_unmap_close(finalizer->cbuffer, finalizer->buffer_name);
    if (!ret) {
        exit(EXIT_FAILURE);
    }

    //Unmap system state
    ret = sys_state_unmap_close(finalizer->sys_state, finalizer->buffer_name);
    if (!ret) {
        exit(EXIT_FAILURE);
    }

    // Get current time
    ret = gettimeofday(&end_all, NULL);
    if (ret) {
        fprintf(stderr,
                "\nFinalizer PID: %u for buffer: %s failed to get current time\n",
                finalizer->process_id,
                finalizer->buffer_name);
        return ret;
    }

    // Compute whole time
    time_interval = get_time_interval(start_all, end_all);
    finalizer->time_elapsed.tv_sec = time_interval.tv_sec;
    finalizer->time_elapsed.tv_usec = time_interval.tv_usec;
    

    //STATS
    fprintf(stdout,
            "\n Finalizer PID: %u for buffer: %s has finalized {\n",
            finalizer->process_id, finalizer->buffer_name);
    fprintf(stdout," Producers counter %u\n", producer_count);
    fprintf(stdout,
            " Finalizer messages counter: %u\n", finalizer->message_count);
    fprintf(stdout,
            " Finalizer accumulated blocked time by cbuffer empty space semaphore: %lu seconds and %lu milliseconds\n",
            finalizer->blocked_time_by_empty_sem_s.tv_sec, finalizer->blocked_time_by_empty_sem_s.tv_usec);
    finalizer->blocked_time_by_wr_mut_s = format_accumulated_time(finalizer->blocked_time_by_wr_mut_s);
    fprintf(stdout,
            " Finalizer accumulated blocked time by cbuffer write mutex: %lu seconds and %lu milliseconds\n",
           finalizer->blocked_time_by_wr_mut_s.tv_sec, finalizer->blocked_time_by_wr_mut_s.tv_usec);
    fprintf(stdout,
            " Finalizer total consumer waiting time: %lu seconds and %lu milliseconds\n",
           finalizer->waiting_consumers.tv_sec, finalizer->waiting_consumers.tv_usec);
    fprintf(stdout,
            " Finalizer total producer waiting time: %lu seconds and %lu milliseconds\n",
           finalizer->waiting_producers.tv_sec, finalizer->waiting_producers.tv_usec);
    fprintf(stdout,
            " Finalizer total running time: %lu seconds and %lu milliseconds\n",
           finalizer->time_elapsed.tv_sec, finalizer->time_elapsed.tv_usec);
    fprintf(stdout, "}\n");

    return ret;
}