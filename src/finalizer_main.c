#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "finalizer.h"
#include "utils.h"

int main(int argc, char *argv[]) {
  int opt;
  char * buffer_name = NULL;
  system_sh_state_t *system_state;

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

  system_state = shm_system_state_get(buffer_name);
  if (!system_state) {
    exit(EXIT_FAILURE);
  }

  system_state->keep_alive = false;

  exit(EXIT_SUCCESS);
}
