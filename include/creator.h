#ifndef PROYECTO1_CREATOR_H
#define PROYECTO1_CREATOR_H


#define N 5
#define PRODUCERS 2
#define CONSUMERS 4
#define SHARED_PROCESS 1
#define SHARED_THREATS 0
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
  /*atomically counts the number of data items in the buffer */
  sem_t data;
  /*atomically counts the empty slots in the buffer (N)*/
  sem_t empty; //N;

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


//Creator dummy stuff
char msg[10];
int print_msg();


#endif //PROYECTO1_CREATOR_H
