#include <stdio.h>
#include <unistd.h>
#include "consumer.h"
#include "buffer.h"
#include "utils.h"

int new_consumer(consumer_t *consumer, char *buffer_name, int mean_s)
{
    consumer->process_id = getpid();
    consumer->buffer_name = buffer_name;
    consumer->exp_mean_wait_s = mean_s;
    consumer->message_count = 0;
    consumer->waited_time_s = 0;
    consumer->blocked_time_s = 0;
    consumer->sys_state = shm_system_state_get(buffer_name);
    if (!consumer->sys_state) return EXIT_FAILURE;

    consumer->cbuffer = shm_cbuffer_get(buffer_name,
                                        consumer->sys_state->buffer_size);
    if (!consumer->cbuffer) return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

int run_consumer(consumer_t *consumer)
{

    int ret;
    message_t message;
    unsigned int wait_time_s;
    bool system_alive;

    // Lock keep alive mutex
    ret = sem_wait(&consumer->sys_state->mut_keep_alive);
    if (ret) {
        fprintf(stdout,
                "Creator PID: %u failed to lock keep alive: %s\n",
                consumer->process_id,
                consumer->buffer_name);
        return ret;
    }

    system_alive = consumer->sys_state->keep_alive;

    // Unlock keep alive mutex
    ret = sem_post(&consumer->sys_state->mut_keep_alive);
    if (ret) {
        fprintf(stdout,
                "Creator PID: %u failed to unlock keep alive: %s\n",
                consumer->process_id,
                consumer->buffer_name);
        return ret;
    }

    while(system_alive) {
        // Consumer waits for a random exponential time according the given mean
        wait_time_s = exponential_random_get(consumer->exp_mean_wait_s);
        fprintf(stdout,
                "\nConsumer PID: %u waiting for %u seconds\n",
                consumer->process_id,
                wait_time_s);
        consumer->waited_time_s += wait_time_s;
        sleep(wait_time_s);

        // Wait message semaphore
        ret = sem_wait(&consumer->sys_state->sem_cbuffer_message);
        if (ret) {
            fprintf(stdout,
                    "\nConsumer PID: %u failed to wait message space: %s\n",
                    consumer->process_id,
                    consumer->buffer_name);
            return ret;
        }

        // Lock cbuffer read mutex
        ret = sem_wait(&consumer->sys_state->mut_cbuffer_read);
        if (ret) {
            fprintf(stdout,
                    "\nConsumer PID: %u failed to lock cbuffer write: %s\n",
                    consumer->process_id,
                    consumer->buffer_name);
        }

        // Read message into shared buffer
        ret = circular_buffer_get(consumer->cbuffer, &message);
        if (ret < 0) {
            fprintf(stdout,
                    "\nConsumer PID: %u failed to read into buffer: %s\n",
                    consumer->process_id,
                    consumer->buffer_name);
            return ret;
        }

        // Unlock cbuffer read mutex
        ret = sem_post(&consumer->sys_state->mut_cbuffer_read);
        if (ret) {
            fprintf(stdout,
                    "\nConsumer PID: %u failed to unlock cbuffer write: %s\n",
                    consumer->process_id,
                    consumer->buffer_name);
            return ret;
        }

        // Post empty semaphore
        ret = sem_post(&consumer->sys_state->sem_cbuffer_empty);
        if (ret) {
            fprintf(stdout,
                    "\nConsumer PID: %u failed to post empty space: %s\n",
                    consumer->process_id,
                    consumer->buffer_name);
            return ret;
        }

        // Print message
        fprintf(stdout, "\nBuffer: %s was read\n", consumer->buffer_name);
        message_print(message);

        consumer->message_count++;
        if (message.consumer_key == consumer->process_id_mod_5){
            printf("Message key (%u) equals PID (%u) mod 5\n", message.consumer_key, consumer->process_id);
            break;
        }

        // Lock keep alive mutex
        ret = sem_wait(&consumer->sys_state->mut_keep_alive);
        if (ret) {
            fprintf(stdout,
                    "Consumer PID: %u failed to lock keep alive: %s\n",
                    consumer->process_id,
                    consumer->buffer_name);
        }

        system_alive = consumer->sys_state->keep_alive;

        // Unlock keep alive mutex
        ret = sem_post(&consumer->sys_state->mut_keep_alive);
        if (ret) {
            fprintf(stdout,
                    "Consumer PID: %u failed to unlock keep alive: %s\n",
                    consumer->process_id,
                    consumer->buffer_name);
            return ret;
        }
    }
    printf("-------------------- FINALIZATION --------------------\n");
    printf("The consumer with process id %d has finalized\n", consumer->process_id);
    printf("Number of consumed messages: %d\n", consumer->message_count);
    printf("Accumulated waiting time: %d seconds\n", consumer->waited_time_s);
    printf("Accumulated time blocked by semaphores: %d seconds\n", consumer->blocked_time_s);
    printf("------------------------------------------------------\n");
    return ret;
}