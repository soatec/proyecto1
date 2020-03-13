#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include "producer.h"

int new_producer(producer_t *producer, char *buffer_name, unsigned int exp_mean)
{
    producer->process_id = getpid();
    producer->buffer_name = buffer_name;
    producer->exp_mean_wait_s = exp_mean;
    producer->message_count = 0;
    producer->waited_time_s = 0;
    producer->blocked_time_by_empty_sem_s = (struct timeval){0};
    producer->blocked_time_by_wr_mut_s = (struct timeval){0};

    producer->sys_state = shm_system_state_get(buffer_name);
    if (!producer->sys_state) return EXIT_FAILURE;

    producer->cbuffer = shm_cbuffer_get(buffer_name,
                                        producer->sys_state->buffer_size,
                                        producer->sys_state->cbuffer_address);
    if (!producer->cbuffer) return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

int run_producer(producer_t *producer)
{
    int ret;
    message_t message;
    struct timeval start_time, end_time, time_interval;
    unsigned int wait_time_s, cbuffer_index;
    unsigned int producer_count, consumer_count;

    // Set producer PID int message
    message.process_id = producer->process_id;

    // Lock producer count mutex
    ret = sem_wait(&producer->sys_state->mut_producer_count);
    if (ret) {
        fprintf(stderr,
                "\nProducer PID: %u for buffer: %s failed to lock producer count\n",
                producer->process_id,
                producer->buffer_name);
        return ret;
    }

    // Increment producer counter
     producer->sys_state->producer_count += 1;

    // Unlock producer count mutex
    ret = sem_post(&producer->sys_state->mut_producer_count);
    if (ret) {
        fprintf(stderr,
                "\nProducer PID: %u for buffer: %s failed to unlock producer count\n",
                producer->process_id,
                producer->buffer_name);
        return ret;
    }

    fprintf(stderr,
            "\nProducer PID: %u for buffer: %s created\n",
            producer->process_id,
            producer->buffer_name);

    while(producer->sys_state->keep_alive) {
        // Producer waits for a random exponential time according the given mean
        wait_time_s = exponential_random_get(producer->exp_mean_wait_s);
        fprintf(stdout,
                "\nProducer PID: %u for buffer: %s waiting for %u seconds\n",
                producer->process_id,
                producer->buffer_name,
                wait_time_s);
        producer->waited_time_s += wait_time_s;
        sleep(wait_time_s);

        // Get current time
        message.time_of_creation = time(NULL);

        // Get consumer key in range [0,4]
        message.consumer_key = rand() % 5;

        // Get current time
        ret = gettimeofday(&start_time, NULL);
        if (ret) {
            fprintf(stderr,
                    "\nProducer PID: %u for buffer: %s failed to get current time\n",
                    producer->process_id,
                    producer->buffer_name);
            return ret;
        }

        // Lock empty semaphore
        ret = sem_wait(&producer->sys_state->sem_cbuffer_empty);
        if (ret) {
            fprintf(stderr,
                  "\nProducer PID: %u for buffer: %s failed to lock empty space\n",
                  producer->process_id,
                  producer->buffer_name);
            return ret;
        }

        // Get current time
        ret = gettimeofday(&end_time, NULL);
        if (ret) {
            fprintf(stderr,
                    "\nProducer PID: %u for buffer: %s failed to get current time\n",
                    producer->process_id,
                    producer->buffer_name);
            return ret;
        }

        // Compute blocked time by cbuffer empty space semaphore
        time_interval = get_time_interval(start_time, end_time);
        producer->blocked_time_by_empty_sem_s.tv_sec += time_interval.tv_sec;
        producer->blocked_time_by_empty_sem_s.tv_usec += time_interval.tv_usec;

        // Get current time
        ret = gettimeofday(&start_time, NULL);
        if (ret) {
            fprintf(stderr,
                    "\nProducer PID: %u for buffer: %s failed to get current time\n",
                    producer->process_id,
                    producer->buffer_name);
            return ret;
        }

        // Lock cbuffer write mutex
        ret = sem_wait(&producer->sys_state->mut_cbuffer_write);
        if (ret) {
            fprintf(stderr,
                    "\nProducer PID: %u for buffer: %s failed to lock cbuffer write\n",
                    producer->process_id,
                    producer->buffer_name);
            return ret;
        }

        // Get current time
        ret = gettimeofday(&end_time, NULL);
        if (ret) {
            fprintf(stderr,
                    "\nProducer PID: %u for buffer: %s failed to get current time\n",
                    producer->process_id,
                    producer->buffer_name);
            return ret;
        }

        // Compute blocked time by cbuffer write mutex
        time_interval = get_time_interval(start_time, end_time);
        producer->blocked_time_by_wr_mut_s.tv_sec += time_interval.tv_sec;
        producer->blocked_time_by_wr_mut_s.tv_usec += time_interval.tv_usec;

        // Write message into shared buffer
        cbuffer_index = circular_buffer_put(producer->cbuffer, message);
        if (cbuffer_index < 0) {
            fprintf(stderr,
                    "\nProducer PID: %u for buffer: %s failed to write into buffer\n",
                    producer->process_id,
                    producer->buffer_name);
            ret = EXIT_FAILURE;
            return ret;
        }

        // Unlock cbuffer write mutex
        ret = sem_post(&producer->sys_state->mut_cbuffer_write);
        if (ret) {
            fprintf(stderr,
                  "\nProducer PID: %u for buffer: %s failed to unlock cbuffer write\n",
                  producer->process_id,
                  producer->buffer_name);
            return ret;
        }

        // Post message semaphore
        ret = sem_post(&producer->sys_state->sem_cbuffer_message);
        if (ret) {
            fprintf(stderr,
                    "\nProducer PID: %u for buffer: %s failed to post message semaphore\n",
                    producer->process_id,
                    producer->buffer_name);
            return ret;
        }

        // Get producers and consumers count
        producer_count = producer->sys_state->producer_count;
        consumer_count = producer->sys_state->consumer_count;

        // Increment produced messages count
        producer->message_count += 1;

        // Print message
        fprintf(stdout, "\nBuffer: %s was written at index %u\n"
                " Producers counter %u, Consumers counter %u\n",
                producer->buffer_name, cbuffer_index, producer_count,
                consumer_count);
        message_print(message);
    }

    // Lock producer count mutex
    ret = sem_wait(&producer->sys_state->mut_producer_count);
    if (ret) {
        fprintf(stderr,
                "\nProducer PID: %u for buffer: %s failed to lock producer count\n",
                producer->process_id,
                producer->buffer_name);
        return ret;
    }

    // Decrement producer counter
    producer->sys_state->producer_count -= 1;
    producer_count = producer->sys_state->producer_count;

    // Unlock producer count mutex
    ret = sem_post(&producer->sys_state->mut_producer_count);
    if (ret) {
        fprintf(stderr,
                "\nProducer PID: %u for buffer: %s failed to unlock producer count\n",
                producer->process_id,
                producer->buffer_name);
        return ret;
    }
    fprintf(stdout,
            "\n Producer PID: %u for buffer: %s has finalized {\n",
            producer->process_id, producer->buffer_name);
    fprintf(stdout," Producers counter %u\n", producer_count);
    fprintf(stdout,
            " Produced messages counter: %u\n", producer->message_count);
    fprintf(stdout,
            " Accumulated waiting time: %u seconds\n", producer->waited_time_s);
    producer->blocked_time_by_empty_sem_s = format_accumulated_time(producer->blocked_time_by_empty_sem_s);
    fprintf(stdout,
            " Accumulated blocked time by cbuffer empty space semaphore: %lu seconds and %lu milliseconds\n",
            producer->blocked_time_by_empty_sem_s.tv_sec, producer->blocked_time_by_empty_sem_s.tv_usec);
    producer->blocked_time_by_wr_mut_s = format_accumulated_time(producer->blocked_time_by_wr_mut_s);
    fprintf(stdout,
            " Accumulated blocked time by cbuffer write mutex: %lu seconds and %lu milliseconds\n",
           producer->blocked_time_by_wr_mut_s.tv_sec, producer->blocked_time_by_wr_mut_s.tv_usec);
    fprintf(stdout, "}\n");

    return ret;
}
