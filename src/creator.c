#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../include/creator.h"
#include "../include/buffer.h"
#include <sys/shm.h> //Memoria compartida
#include <sys/ipc.h>
#include <pthread.h>   /* pthread_... */
#include <semaphore.h> /* sem_... */

#define N 5
#define PRODUCERS 2
#define CONSUMERS 4
/*
este es el slot de struct usada como buffer (muts deleate later)
*/
typedef struct {
  char *name;
  char id[N];

} item;
typedef struct {
    item **slots;
    /*
    A circular managment:
    use the modulo operador to make the index next_in and next_out
    */
    int capacity;
    int next_in; //keep track of where to produce the next item (N-1)
    int next_out;//keep track of where to consume the next item (N-3)
} buffer_head;


/*******************************************************************
* NAME : access_to_buffer_struct
* DESCRIPTION :
*           this struct implements the emaphores used
*           to synchonize access to the buffer
*
* NOTES :
*/
typedef struct{
  /*mutual exclusive: updates next_in next_out*/
  sem_t mutex;
  int mutex_id;
  /*atomically counts the number of data items in the buffer */
  sem_t data;
  int data_id;
  /*atomically counts the empty slots in the buffer (N)*/
  sem_t empty; //N;
  int empty_id;

} access_to_buffer_struct;


/*******************************************************************
* NAME : readers_and_writers_struct
* DESCRIPTION :
*         this struct implements the semaphores used
*           to synchonize readers and writers
*
* NOTES :
*/
typedef struct{
  /*mutual exclusion amount writers*/
  sem_t wrt;
  /*mutual exclusion uptades of readcount*/
  sem_t mutex;
  int readcount;
}readers_and_writers_struct;


/*******************************************************************
* NAME : init_buffer
* DESCRIPTION :
*     init capacity size of the buffer
*     init next_in
*     init next_out
* INPUTS : n_slots of stols o capacidad el buffer
* OUTPUTS : buffer_head struct
* PROCESS :
* NOTES :
*/
buffer_head * init_buffer(int n_slots){
  buffer_head *buffer = malloc(sizeof *buffer);
    if (!buffer) return 0;
  buffer->capacity = n_slots;
  buffer->next_in = 0;
  buffer->next_out = 0;
  buffer->slots = malloc(n_slots * sizeof *(buffer->slots));
  return buffer;
}

/*******************************************************************
* NAME : init_access_to_buffer
* DESCRIPTION :
*     init mutex for next_in and next_out
*     init numeber of items in the buffer
*     init numbers of stols
* INPUTS : n_buffer or buffer size or number of stols
* OUTPUTS : sync_access_to_buffer struct
* PROCESS :
* NOTES :
*/
access_to_buffer_struct * init_access_to_buffer_struct(int n_buffer){
    access_to_buffer_struct *struct_t;
    int id;
    struct_t = (access_to_buffer_struct *) malloc(sizeof(access_to_buffer_struct));
    id = sem_init(&(struct_t->mutex), 0, 1);
    struct_t->mutex_id = id;
    id = sem_init(&(struct_t->data), 0, 0);
    struct_t->data_id = id;
    id = sem_init(&(struct_t->empty), 0, n_buffer);
    struct_t->empty_id = id;
    //struct_t = { .mutex= 1, .data = 0, .empy = n_buffer};//mutex,data,empty
    return struct_t;
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
readers_and_writers_struct * init_readers_and_writers_struct (){
    //sync_readers_and_writers struct_t p1 = {1,1};//wrt, mutex
    readers_and_writers_struct *struct_t;
    struct_t = (readers_and_writers_struct *) malloc(sizeof(readers_and_writers_struct));
    sem_init(&(struct_t->wrt), 0, 1);
    sem_init(&(struct_t->mutex), 0, 1);
    struct_t->readcount = 0;
    return struct_t;
}

buffer_head *buffer;
access_to_buffer_struct *access_to_buffer_head;
readers_and_writers_struct *readers_and_writers_t;

pthread_t consumer_tid[CONSUMERS], producer_tid[PRODUCERS];



int print_msg()
{
    strcpy(msg, "Creator");
    //printf("loaded >>> %s %i \n", msg, get_buffer_int());
    printf("\033[1;33m");
    printf("inicia el buffer\n");
    buffer  = init_buffer(N);

    printf("init semaphores\n");
    access_to_buffer_head = init_access_to_buffer_struct(N);
    readers_and_writers_t =init_readers_and_writers_struct ();
    printf("\033[0m");
    return 0;
}

int main() {

  print_msg();



  return(0);
}
