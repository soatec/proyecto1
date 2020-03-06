#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../include/buffer.h"

int buffer_int = 10;

int get_buffer_int()
{
    return buffer_int;
}



circular_buffer_t* circular_buffer_init(size_t size)
{
    circular_buffer_t* circular_buffer = (circular_buffer_t *) malloc(sizeof(circular_buffer_t));
    circular_buffer->buffer = (message_t *) malloc (sizeof(message_t)*size);
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