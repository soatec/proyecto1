#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../include/consumer.h"
#include "../include/buffer.h"

int print_msg()
{
    strcpy(msg, "Consumer");
    printf("loaded >>> %s %i \n", msg, get_buffer_int());
    return 0;
}