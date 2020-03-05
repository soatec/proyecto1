#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../include/creator.h"
#include "../include/buffer.h"
#include <sys/shm.h> //Memoria compartida
#include <sys/ipc.h>

#define BUFFER 1024

int print_msg()
{
    strcpy(msg, "Creator");
    printf("loaded >>> %s %i \n", msg, get_buffer_int());
    return 0;
}


int int createMemoryShared(){
    int pid,memoriaID;
    char *punteroAMemoriaCompartida = NULL;
    puts("creo memoria compartida para varios procesos");

  //trabajando en esto... esta  linea es diferente
    if((memoriaID = shmget(1315511,BUFFER,0664|IPC_CREAT))==-1) {
      //El primer valor es un identificador unico hay que cambiar por el espacioquevamos a usar
        fprintf(stderr,"Error al reservar la memoria");
    } //Creo la memoria compartida

    pid = fork();
    switch(pid) {
        case -1:
            fprintf(stderr,"Error al hacer el fork");
        break;
        case 0: //El hijo
            punteroAMemoriaCompartida = shmat(memoriaID,(void *)0,0);
            //Asociacia a la memoria compartida
            puts("Soy el hijo ");
            // aqui tengo que inicializar otros valores

        break;
        default:

            punteroAMemoriaCompartida = shmat(memoriaID,NULL,0); //Asociacion
            puts("Soy el padre");
          //aqui tengo que inicialzar igual otros valores

        break;
    }
    return 0;
}

int freeSharedMemory(punteroAMemoriaCompartida){
  shmdt(&punteroAMemoriaCompartida); //Desasociacion
  if(shmctl(memoriaID,IPC_RMID,NULL)==-1){
      fprintf(stderr,"Error al liberar la memoria");
  }
  return 0;
}
