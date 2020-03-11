#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <pthread.h>   /* pthread_... */
#include <semaphore.h> /* sem_... */
#include <sys/mman.h>
#include <fcntl.h>
#include "../include/buffer.h"
#include "../include/creator.h"

/*
A ring(or circular) buffer is a fixed-size buffer,
which can overwrite new data to the beginning of the buffer when the buffer is full.
The ring buffer can be simply implemented with an array and two indices
  - one index points the beginning of the buffer,
    and the other index denotes the end of the buffer.

If the buffer is empty, the begin index equals to the end index(for simplicity, set them to 0).
    When new data comes, push it to the entry of the end index,
    and increase the end index.

    At this moment, if the begin index equals to the end index,
      then increase the begin index, too.

    All of the index operations must modulo the size of the array, in case of buffer overflow.

    reference : https://www.bo-yang.net/2016/07/27/shared-memory-ring-buffer
*/


/*******************************************************************
* NAME : init_buffer
* DESCRIPTION :
*     init capacity size of the buffer
*     init next_in
*     init next_out
* INPUTS : n_buffer of stols o capacidad el buffer
* OUTPUTS : circular_buffer_t struct
* PROCESS : If the buffer is empty, the begin index (update next_in)
*                         equals to the end index (update next_out)
*           (for simplicity, set them to 0).
* NOTES :
*/
/*
circular_buffer_t * init_buffer(int n_buffer){
  circular_buffer_t *buffer = malloc(sizeof *buffer);
    if (!buffer) return 0;
  buffer->capacity = n_buffer;
  buffer->next_in = 0;//keep track of where to produce the next message_t (N-1)
  buffer->next_out = 0;//keep track of where to consume the next message_t (N-3)
  buffer->buffer = malloc(n_buffer * sizeof *(buffer->buffer));
  return buffer;
}
*/
/*******************************************************************
* NAME : init_access_to_buffer
* DESCRIPTION :
*     init mutex for next_in and next_out
*     init numeber of message_ts in the buffer
*     init numbers of stols
* INPUTS : n_buffer or buffer size or number of stols
* OUTPUTS : sync_access_to_buffer struct
* PROCESS : empty count the empty buffer in the buffer and it's initialized with the total value.
* NOTES :
*/
void init_sem_lock_buffer(int n_buffer,struct sem_lock_buffer *struct_t){
    sem_init(&(struct_t->mutex), SHARED_PROCESS, 1);
    sem_init(&(struct_t->data), SHARED_PROCESS, 0);// count the number of data message_ts in the buffer
    sem_init(&(struct_t->empty), SHARED_PROCESS, n_buffer);//count the empty slot in the buffer
    //struct_t = { .mutex= 1, .data = 0, .empy = n_buffer};//mutex,data,empty
}


/*******************************************************************
* NAME : init_access_to_buffer
* DESCRIPTION :
*     init wrt mutual exclusion amount writers
*     init mutual exclusion uptades of readcount
* INPUTS :
* OUTPUTS : sync_readers_and_writers struct
* PROCESS :
* NOTES :
*/
void init_sem_look_read_write (struct sem_look_read_write *struct_t){
    //sync_readers_and_writers struct_t p1 = {1,1};//wrt, mutex
    sem_init(&(struct_t->wrt), SHARED_PROCESS, 1);
    sem_init(&(struct_t->mutex), SHARED_PROCESS, 1);
    struct_t->readcount = 0;
}


/*******************************************************************
* NAME : add_data_index_buffer
* DESCRIPTION :
*    The code for adding new data into the buffer, is like to write information
* into the buffer.
*   update values of next_in next_out in order to manage the buffer
* INPUTS :
* OUTPUTS :
* PROCESS : A - ring buffer array
*             next_in - start index of the ring buffer
*             next_out - end index of the ring buffer
*             capacity - size of the ring buffer
* NOTES :
*/
/*
circular_buffer_t*  add_data_index_buffer(message_t  data_message_t, circular_buffer_t *buffer){
  change this line for add new information into the buffer
  buffer->buffer[buffer->next_in]=data_message_t;
  buffer->next_in = (buffer->next_in + 1)% buffer->capacity;
  if (buffer->next_out == buffer->next_in);
    buffer->next_out = (buffer->next_out + 1) % buffer->capacity;
  return buffer;
}
*/

/*******************************************************************
* NAME : remove_data_index_buffer
* DESCRIPTION :
*    The code for adding new data into the buffer, is like to write information
* into the buffer.
*   update values of next_in next_out in order to manage the buffer
* INPUTS :
* OUTPUTS :
* PROCESS : A - ring buffer array
*             next_in - start index of the ring buffer
*             next_out - end index of the ring buffer
*             capacity - size of the ring buffer
* NOTES :
*/
/*
circular_buffer_t*  remove_data_index_buffer(message_t  data_message_t, circular_buffer_t *buffer){
  change this line for add new information into the buffer
  buffer->buffer[buffer->next_in]=data_message_t;
  buffer->next_out = (buffer->next_out + 1)% buffer->capacity;
  if (buffer->next_out == buffer->next_in);
    buffer->next_in = (buffer->next_in + 1) % buffer->capacity;
  return buffer;
}
*/
/*******************************************************************
* NAME : create_shared_mmap
* DESCRIPTION :
*
* into the buffer.
*
* INPUTS :
* OUTPUTS :
* PROCESS :
* NOTES :
*/
int create_shared_mmap(char* name, int _capacity){
    int shm_fd;
    shm_fd = shm_open(name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (shm_fd == -1){
      perror("shm_open failed");
      return 0;
    }

    int shm_len;
    // = malloc(sizeof(struct sem_lock_buffer) + sizeof(struct sem_look_read_write) + sizeof(struct circular_buffer_t) +  _capacity * sizeof(struct message_t));
    if (ftruncate(shm_fd, shm_len) < 0) {
        perror("ftruncate failed");
        //shm_unlink(path);
        //return false;
    }

    void* ptr;
    ptr = mmap(0, shm_len, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if ((ptr == (void *) -1)||(ptr == MAP_FAILED)) {
        perror("mmap failed");
        return 0;
    }


    printf("%s", (char*)ptr);

      /* Unmap shared memory */
      munmap(ptr, shm_len);
      /* Destroy shared memory */
      shm_unlink(name);

    return 0;
}

int creator_of_values()
{
    strcpy(msg, "Creator");
    //printf("loaded >>> %s %i \n", msg, get_buffer_int());
    printf("\033[1;33m");
    printf("init the buffer\n");
    struct circular_buffer_t buffer_t;
    //buffer  = init_buffer(CAPACITY);

    printf("init semaphores to manage buffer \n");
    struct sem_lock_buffer sem_lock_buffer_t;
    init_sem_lock_buffer(CAPACITY,&sem_lock_buffer_t);

    printf("init semaphores to manage productor consumitor \n");
    struct sem_look_read_write sem_lock_rw_t;
    init_sem_look_read_write (&sem_lock_rw_t);
    create_shared_mmap(BUFFER_NAME,CAPACITY);

    
    printf("\033[0m");
    return 0;
}
