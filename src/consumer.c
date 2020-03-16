#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include "consumer.h"
#include "buffer.h"
#include "utils.h"

int new_consumer(consumer_t *consumer, char *buffer_name, int mean_s)
{
    consumer->process_id = getpid();
    consumer->process_id_mod_5 = consumer->process_id % 5;
    consumer->buffer_name = buffer_name;
    consumer->exp_mean_wait_s = mean_s;
    consumer->message_count = 0;
    consumer->waited_time_s = 0;
    consumer->blocked_time_by_message_sem_s = (struct timeval){0};
    consumer->blocked_time_by_rd_mut_s = (struct timeval){0};
    consumer->sys_state = shm_system_state_get(buffer_name);
    if (!consumer->sys_state) return EXIT_FAILURE;
    consumer->cbuffer = shm_cbuffer_get(buffer_name,
                                        consumer->sys_state->buffer_size,
                                        consumer->sys_state->cbuffer_address);
    if (!consumer->cbuffer) return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

int run_consumer(consumer_t *consumer)
{

    int ret;
    message_t message;
    struct timeval start_time, end_time, time_interval;
    unsigned int wait_time_s, cbuffer_index;
    bool system_alive = consumer->sys_state->keep_alive;
    unsigned int producer_count, consumer_count;

    // Lock consumer count mutex
    ret = sem_wait(&consumer->sys_state->mut_consumer_count);
    if (ret) {
        fprintf(stderr,
                "\nConsumer PID: %u for buffer: %s failed to lock consumer count\n",
                consumer->process_id,
                consumer->buffer_name);
    }

    // Increment consumer counter
    consumer->sys_state->consumer_count += 1;

    // Unlock consumer count mutex
    ret = sem_post(&consumer->sys_state->mut_consumer_count);
    if (ret) {
        fprintf(stderr,
                "\nConsumer PID: %u for buffer: %s failed to unlock consumer count\n",
                consumer->process_id,
                consumer->buffer_name);
        return ret;
    }

    fprintf(stderr,
            "\nConsumer PID: %u for buffer: %s created\n",
            consumer->process_id,
            consumer->buffer_name);


    while(system_alive) {
        // Consumer waits for a random exponential time according the given mean
        wait_time_s = exponential_random_get(consumer->exp_mean_wait_s);
        fprintf(stdout,
                "\nConsumer PID: %u waiting for %u seconds\n",
                consumer->process_id,
                wait_time_s);
        consumer->waited_time_s += wait_time_s;
        sleep(wait_time_s);

        // Get current time
        ret = gettimeofday(&start_time, NULL);
        if (ret) {
            fprintf(stderr,
                    "\nConsumer PID: %u for buffer: %s failed to get current time\n",
                    consumer->process_id,
                    consumer->buffer_name);
            return ret;
        }

        // Wait message semaphore
        ret = sem_wait(&consumer->sys_state->sem_cbuffer_message);
        if (ret) {
            fprintf(stdout,
                    "\nConsumer PID: %u failed to wait message space: %s\n",
                    consumer->process_id,
                    consumer->buffer_name);
            return ret;
        }

        // Get current time
        ret = gettimeofday(&end_time, NULL);
        if (ret) {
            fprintf(stderr,
                    "\nConsumer PID: %u for buffer: %s failed to get current time\n",
                    consumer->process_id,
                    consumer->buffer_name);
            return ret;
        }

        // Compute blocked time by cbuffer message semaphore
        time_interval = get_time_interval(start_time, end_time);
        consumer->blocked_time_by_message_sem_s.tv_sec += time_interval.tv_sec;
        consumer->blocked_time_by_message_sem_s.tv_usec += time_interval.tv_usec;

        // Get current time
        ret = gettimeofday(&start_time, NULL);
        if (ret) {
            fprintf(stderr,
                    "\nConsumer PID: %u for buffer: %s failed to get current time\n",
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

        // Get current time
        ret = gettimeofday(&end_time, NULL);
        if (ret) {
            fprintf(stderr,
                    "\nConsumer PID: %u for buffer: %s failed to get current time\n",
                    consumer->process_id,
                    consumer->buffer_name);
            return ret;
        }

        // Compute blocked time by cbuffer write mutex
        time_interval = get_time_interval(start_time, end_time);
        consumer->blocked_time_by_rd_mut_s.tv_sec += time_interval.tv_sec;
        consumer->blocked_time_by_rd_mut_s.tv_usec += time_interval.tv_usec;

        // Read message into shared buffer
        cbuffer_index = circular_buffer_get(consumer->cbuffer, &message);
        if (cbuffer_index < 0) {
            fprintf(stdout,
                    "\nConsumer PID: %u failed to read into buffer: %s\n",
                    consumer->process_id,
                    consumer->buffer_name);
            return EXIT_FAILURE;
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

        producer_count = consumer->sys_state->producer_count;
        consumer_count = consumer->sys_state->consumer_count;

        // Print message
        fprintf(stdout, "\nBuffer: %s was read at index %u\n"
                        " Producers counter %u, Consumers counter %u\n",
                consumer->buffer_name, cbuffer_index, producer_count,
                consumer_count);
        message_print(message);

        consumer->message_count++;
        if (message.consumer_key == consumer->process_id_mod_5){
            printf("Message key (%u) equals PID (%u) mod 5\n", message.consumer_key, consumer->process_id);
            break;
        }

        if (message.consumer_key == -1){
            printf("Finalize message recieved.\n");
            break;
        }

        system_alive = consumer->sys_state->keep_alive;

    }

    // Lock consumer count mutex
    ret = sem_wait(&consumer->sys_state->mut_consumer_count);
    if (ret) {
        fprintf(stderr,
                "\nConsumer PID: %u for buffer: %s failed to lock consumer count\n",
                consumer->process_id,
                consumer->buffer_name);
    }

    // Decrement consumer counter
    consumer->sys_state->consumer_count -= 1;
    consumer_count = consumer->sys_state->consumer_count;

    // Unlock consumer count mutex
    ret = sem_post(&consumer->sys_state->mut_consumer_count);
    if (ret) {
        fprintf(stderr,
                "\nConsumer PID: %u for buffer: %s failed to unlock consumer count\n",
                consumer->process_id,
                consumer->buffer_name);
        return ret;
    }

    fprintf(stdout,
            "\n Consumer PID: %u for buffer: %s has finalized {\n",
            consumer->process_id, consumer->buffer_name);
    fprintf(stdout," Consumer counter %u\n", consumer_count);
    fprintf(stdout,
            " Consumed messages counter: %u\n", consumer->message_count);
    fprintf(stdout,
            " Accumulated waiting time: %u seconds\n", consumer->waited_time_s);
    consumer->blocked_time_by_message_sem_s = format_accumulated_time(consumer->blocked_time_by_message_sem_s);
    fprintf(stdout,
            " Accumulated blocked time by cbuffer message semaphore: %lu seconds and %lu milliseconds\n",
            consumer->blocked_time_by_message_sem_s.tv_sec, consumer->blocked_time_by_message_sem_s.tv_usec);
    consumer->blocked_time_by_rd_mut_s = format_accumulated_time(consumer->blocked_time_by_rd_mut_s);
    fprintf(stdout,
            " Accumulated blocked time by cbuffer read mutex: %lu seconds and %lu milliseconds\n",
            consumer->blocked_time_by_rd_mut_s.tv_sec, consumer->blocked_time_by_rd_mut_s.tv_usec);
    fprintf(stdout, "}\n");
    return ret;
}
