#include <stdio.h>
#include <string.h>
#include "finalizer.h"
#include "buffer.h"

int print_msg()
{
    strcpy(msg, "Finalizer");
    printf("loaded >>> %s %i \n", msg, get_buffer_int());
    return 0;
}