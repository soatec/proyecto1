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
    producer->blocked_time_s = 0;
    producer->sys_state = shm_system_state_get(buffer_name);
    if (!producer->sys_state) return EXIT_FAILURE;

    producer->cbuffer = shm_cbuffer_get(buffer_name,
                                        producer->sys_state->buffer_size);
    if (!producer->cbuffer) return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

int run_producer(producer_t *producer)
{
    int ret;
    message_t message;
    unsigned int wait_time_s;
    bool system_alive;

    // Set producer PID int message
    message.process_id = producer->process_id;

    // Lock keep alive mutex
    ret = sem_wait(&producer->sys_state->mut_keep_alive);
    if (ret) {
        fprintf(stdout,
                "Creator PID: %u failed to lock keep alive: %s\n",
                producer->process_id,
                producer->buffer_name);
    }

    system_alive = producer->sys_state->keep_alive;

    // Unlock keep alive mutex
    ret = sem_post(&producer->sys_state->mut_keep_alive);
    if (ret) {
        fprintf(stdout,
              "Creator PID: %u failed to unlock keep alive: %s\n",
              producer->process_id,
              producer->buffer_name);
        return ret;
    }

    while(system_alive) {
        // Producer waits for a random exponential time according the given mean
        wait_time_s = exponential_random_get(producer->exp_mean_wait_s);
        fprintf(stdout,
                "\nProducer PID: %u waiting for %u seconds\n",
                producer->process_id,
                wait_time_s);
        producer->waited_time_s += wait_time_s;
        sleep(wait_time_s);

        // Get current time
        message.time_of_creation = time(NULL);

        // Get consumer key in range [0,4]
        message.consumer_key = rand() % 5;

        // Wait empty semaphore
        ret = sem_wait(&producer->sys_state->sem_cbuffer_empty);
        if (ret) {
            fprintf(stdout,
                  "\nProducer PID: %u failed to lock empty space: %s\n",
                  producer->process_id,
                  producer->buffer_name);
            return ret;
        }

        // Lock cbuffer write mutex
        ret = sem_wait(&producer->sys_state->mut_cbuffer_write);
        if (ret) {
            fprintf(stdout,
                    "\nProducer PID: %u failed to lock cbuffer write: %s\n",
                    producer->process_id,
                    producer->buffer_name);
        }

        // Write message into shared buffer
        ret = circular_buffer_put(producer->cbuffer, message);
        if (ret < 0) {
            fprintf(stdout,
                    "\nProducer PID: %u failed to write into buffer: %s\n",
                    producer->process_id,
                    producer->buffer_name);
            return ret;
        }

        // Unlock cbuffer write mutex
        ret = sem_post(&producer->sys_state->mut_cbuffer_write);
        if (ret) {
            fprintf(stdout,
                  "\nProducer PID: %u failed to unlock cbuffer write: %s\n",
                  producer->process_id,
                  producer->buffer_name);
            return ret;
        }

        // Post message semaphore
        ret = sem_post(&producer->sys_state->sem_cbuffer_message);
        if (ret) {
            fprintf(stdout,
                    "\nProducer PID: %u failed to post message semaphore: %s\n",
                    producer->process_id,
                    producer->buffer_name);
            return ret;
        }

        // Print message
        fprintf(stdout, "\nBuffer: %s was written\n", producer->buffer_name);
        message_print(message);

        // Lock keep alive mutex
        ret = sem_wait(&producer->sys_state->mut_keep_alive);
        if (ret) {
            fprintf(stdout,
                    "Producer PID: %u failed to lock keep alive: %s\n",
                    producer->process_id,
                    producer->buffer_name);
        }

        system_alive = producer->sys_state->keep_alive;

        // Unlock keep alive mutex
        ret = sem_post(&producer->sys_state->mut_keep_alive);
        if (ret) {
            fprintf(stdout,
                  "Producer PID: %u failed to unlock keep alive: %s\n",
                  producer->process_id,
                  producer->buffer_name);
            return ret;
        }
    }
    return ret;
}
