#ifndef PROYECTO1_CREATOR_H
#define PROYECTO1_CREATOR_H

#include "buffer.h"
#include "utils.h"

/**
 * Initialize shared system memory:
 * - Global variables
 * - Circular buffer
 *
 * @param buffer_name
 * @param buffer_size
 * @param system_state
 * @param cbuffer
 * @return
 */
int new_creator(char* buffer_name, unsigned int buffer_size,
                system_sh_state_t *system_state, circular_buffer_t* cbuffer);

/**
 * Run creator
 *
 * @param system_state
 * @param buffer_name
 * @return
 */
int run_creator(system_sh_state_t *system_state, char* buffer_name);

#endif //PROYECTO1_CREATOR_H
