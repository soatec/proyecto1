#include <time.h>
#include <stdbool.h>

#ifndef PROYECTO1_BUFFER_H
#define PROYECTO1_BUFFER_H

//Buffer dummy code
//Will delete until other dummy refs have been deleted
int get_buffer_int();

//Structs

/**
 * Message structure
 */
typedef struct message_t
{
    /** ID of process that wrote the message **/
    int process_id;

    /** Time when the message was created**/
    time_t time_of_creation;

    /** Random key between 0 and 4 to end consumers with matching key**/
    int consumer_key;
} message_t;

/**
 * Circular buffer structure
 */
typedef struct circular_buffer_t
{
    /** Messages array **/
    message_t* buffer;

    /** Position reference to write **/
    int head;

    /** Position reference to read**/
    int tail;

    /** Indicate if the buffer is full **/
    bool full;

    /** Buffer size **/
    int length;
} circular_buffer_t;

/**
 * Malloc cbuffer and buffer
 *
 * @param size
 * @return
 */
circular_buffer_t* circular_buffer_init(size_t size);

/**
 * Get as param an external buffer
 *
 * @param buffer
 * @param size
 * @return
 */
circular_buffer_t* circular_buffer_init_with_buffer(message_t* buffer, size_t size);

/**
 * Get as param en external buffer AND circular buffer
 *
 * @param cbuffer
 * @param buffer
 * @param size
 */
void circular_buffer_init_with_cbuffer(circular_buffer_t* cbuffer, message_t* buffer, size_t size);

/**
 * Read message from cbuffer and advance tail
 *
 * @param cbuffer
 * @param data
 * @return
 */
int circular_buffer_get(circular_buffer_t* cbuffer, message_t* data);

/**
 * Add message to cbuffer and advance head
 *
 * @param cbuffer
 * @param message
 * @return
 */
int circular_buffer_put(circular_buffer_t* cbuffer, message_t message);

/**
 * Set cbuffer to initial state
 *
 * @param cbuffer
 */
void circular_buffer_reset(circular_buffer_t* cbuffer);

/**
 * Free cbuffer malloc
 *
 * @param cbuffer
 */
void circular_buffer_free(circular_buffer_t* cbuffer);

/**
 * Is cbuffer empty
 *
 * @param cbuffer
 * @return
 */
bool circular_buffer_empty(circular_buffer_t* cbuffer);

/**
 * Is cbuffer full
 *
 * @param cbuffer
 * @return
 */
bool circular_buffer_full(circular_buffer_t* cbuffer);

/**
 * Length of cbuffer
 *
 * @param cbuffer
 * @return
 */
size_t circular_buffer_capacity(circular_buffer_t* cbuffer);

#endif //PROYECTO1_BUFFER_H