#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "finalizer.h"
#include "utils.h"

int main(int argc, char *argv[]) {
  int opt;
  int status;
  char * buffer_name = NULL;
  finalizer_t finalizer;

  // Get buffer name
  while ((opt = getopt(argc, argv, "b:")) != -1) {
      switch (opt) {
          case 'b':
              buffer_name = optarg;
              break;

          default:
              fprintf(stderr, "Usage: %s -b buffer_name\n",
                      argv[0]);
              exit(EXIT_FAILURE);
      }
  }

  if (buffer_name == NULL) {
      fprintf(stderr, "-b buffer_name is a mandatory argument\n");
      exit(EXIT_FAILURE);
  }

  status = new_finalizer(&finalizer, buffer_name);
  if(status){
    fprintf(stderr, "Error while creating finalizer\n");
    return status;
  }

  status = run_finalizer(&finalizer);
    if (status){
        fprintf(stderr, "Error while running finalizer\n");
        return status;
  }
  
  exit(EXIT_SUCCESS);
}
