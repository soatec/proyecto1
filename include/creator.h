#ifndef PROYECTO1_CREATOR_H
#define PROYECTO1_CREATOR_H

/**
 * Initialize shared system memory:
 * - Global variables
 * - Circular buffer
 *
 * @param buffer_name
 * @param buffer_size
 * @return
 */
int shared_system_init(char* buffer_name, unsigned int buffer_size);

#endif //PROYECTO1_CREATOR_H
