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