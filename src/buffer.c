#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include "../include/buffer.h"

//Buffer dummy code
//Will delete until other dummy refs have been deleted
int buffer_int = 10;
int get_buffer_int()
{
    return buffer_int;
}



circular_buffer_t* circular_buffer_init(size_t size)
{
    circular_buffer_t* circular_buffer = (circular_buffer_t *) malloc(sizeof(circular_buffer_t));
    circular_buffer->buffer = (message_t *) malloc (sizeof(message_t)*(size*2));
    if (!circular_buffer->buffer)
    {
        printf("Buffer init failed\n");
    }
    circular_buffer->length = size;
    circular_buffer_reset(circular_buffer);

    return circular_buffer;
}

void circular_buffer_reset(circular_buffer_t* cbuffer)
{
    cbuffer->head = 0;
    cbuffer->tail = 0;
    cbuffer->full = false;
}

void circular_buffer_free(circular_buffer_t* cbuffer)
{
    free(cbuffer);
}

bool circular_buffer_full(circular_buffer_t* cbuffer)
{
    return cbuffer->full;
}

bool circular_buffer_empty(circular_buffer_t* cbuffer)
{
    return (!cbuffer->full && (cbuffer->head == cbuffer->tail));
}

size_t circular_buffer_capacity(circular_buffer_t* cbuffer)
{
    return cbuffer->length;
}

int circular_buffer_put(circular_buffer_t* cbuffer, message_t message)
{
    int result = -1;
    if(!circular_buffer_full(cbuffer))
    {
        cbuffer->buffer[cbuffer->head * sizeof(message_t)] = message;

        result = cbuffer->head;

        cbuffer->head = (cbuffer->head + 1) % cbuffer->length;
        cbuffer->full = (cbuffer->head == cbuffer->tail);

    }
    return result;  //-1 cant insert on full buffer, else, index of inserted message
}

int circular_buffer_get(circular_buffer_t* cbuffer, message_t* data)
{
    int result = -1;
    if(!circular_buffer_empty(cbuffer))
    {
        *data = cbuffer->buffer[cbuffer->tail * sizeof(message_t)];

        result = cbuffer->tail;

        cbuffer->full = false;
        cbuffer->tail = (cbuffer->tail + 1) % cbuffer->length;
    }
    return result;  //-1 cant read on empty buffer, else, index of read message
}