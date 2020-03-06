#include <time.h>
#include <stdbool.h>

#ifndef PROYECTO1_BUFFER_H
#define PROYECTO1_BUFFER_H

//Buffer dummy code
//Will delete until other dummy refs have been deleted
int get_buffer_int();

//Structs
typedef struct message_t
{
  int process_id;   //PID of process that wrote the message.
  time_t time_of_creation;
  int consumer_key;  //Random key between 0 and 4 to end consumers with matching key
} message_t;

typedef struct circular_buffer_t
{
    message_t* buffer;
    int head;
    int tail;
    bool full;
    int length;
} circular_buffer_t;

//Malloc cbuffer and buffer
circular_buffer_t* circular_buffer_init(size_t size);

//Read message from cbuffer and advance tail
int circular_buffer_get(circular_buffer_t* cbuffer, message_t* data);

//Add message to cbuffer and advance head
int circular_buffer_put(circular_buffer_t* cbuffer, message_t message);

//Set cbuffer to initial state
void circular_buffer_reset(circular_buffer_t* cbuffer);

//Free cbuffer malloc
void circular_buffer_free(circular_buffer_t* cbuffer);

//Is cbuffer empty
bool circular_buffer_empty(circular_buffer_t* cbuffer);

//Is cbuffer full
bool circular_buffer_full(circular_buffer_t* cbuffer);

//Length of cbuffer
size_t circular_buffer_capacity(circular_buffer_t* cbuffer);

#endif //PROYECTO1_BUFFER_H
