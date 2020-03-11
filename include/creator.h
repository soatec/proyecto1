#ifndef PROYECTO1_CREATOR_H
#define PROYECTO1_CREATOR_H

#define CAPACITY 5
#define BUFFER_NAME "SO_BUFFER"
#define PRODUCERS 2
#define CONSUMERS 4
#define SHARED_PROCESS 1
#define SHARED_THREATS 0
/*
este es el slot de struct usada como buffer (muts deleate later)
*/

//Structs


/*******************************************************************
* NAME : sem_lock_buffer
* DESCRIPTION :
*           this struct implements the emaphores used
*           to synchonize access to the buffer
*
* NOTES :
*/
struct sem_lock_buffer{
  /*mutual exclusive: updates next_in next_out*/
  sem_t mutex;
  /*atomically counts the number of data message_ts in the buffer */
  sem_t data;
  /*atomically counts the empty buffer in the buffer (N)*/
  sem_t empty; //N;

};


/*******************************************************************
* NAME : sem_look_read_write
* DESCRIPTION :
*         this struct implements the semaphores used
*           to synchonize readers and writers
*
* NOTES :
*/
struct sem_look_read_write{
  /*mutual exclusion amount writers*/
  sem_t wrt;
  /*mutual exclusion uptades of readcount*/
  sem_t mutex;
  int readcount;
};


//Creator dummy stuff
char msg[10];
int print_msg();


#endif //PROYECTO1_CREATOR_H
