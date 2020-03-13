#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "utils.h"

// Preprocessor defines

// System shared state filename sufix
#define SYS_ST_SUFIX "_sys_st"

// Circular buffer filename sufix
#define CBUFFER_SUFIX "_cbuffer"

// Private functions

/*
 * Get filename: Appends name sufix to buffer name
 * The caller is responsible of freeing the returned pointer
 */
static char* shm_filename_get(char* buffer_name, char* name_suffix) {
    char* filename = NULL;

    filename = malloc(sizeof(buffer_name) + sizeof(name_suffix));
    if (!filename) {
        fprintf(stderr, "Failed to allocate sys state file name\n");
        return filename;
    }

    strcpy(filename, buffer_name);
    strcat(filename, name_suffix);

    return filename;
}

// Public functions

unsigned int exponential_random_get(unsigned int mean)
{
    // Get uniform random number in ]0,1[
    double uniform = (rand() + 1) / (RAND_MAX + 2.0);

    // Apply inversion method to get a random exponential number
    return (unsigned int)(-log(1-uniform) * mean);
}

void message_print(message_t message)
{
    fprintf(stdout,
            "Message {\n"
            " PID: %u\n"
            " Creation time: %s"
            " Consumer key: %d\n"
            "}\n",
            message.process_id,
            asctime(localtime(&message.time_of_creation)),
            message.consumer_key);
}

system_sh_state_t *shm_system_state_set(char *buffer_name)
{
    int ret, fd;
    char* filename = NULL;
    system_sh_state_t *system_state = NULL;

    filename = shm_filename_get(buffer_name, SYS_ST_SUFIX);
    if (!filename) {
        return system_state;
    }

    // Open shared memory file: name, oflags, mode
    fd = shm_open(filename, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        fprintf(stderr, "Failed to open sys state shm file\n");
        return system_state;
    }

    // Size shared memory file
    ret = ftruncate(fd, sizeof(system_sh_state_t));
    if (ret) {
        fprintf(stderr, "Failed to truncate sys state shm file\n");
        ret = shm_unlink(filename);
        if (ret) fprintf(stderr, "Failed to unlink sys state shm file\n");
        return system_state;
    }

    // Create sys state mmap: address, size, protection, flags, fd, offset
    system_state = mmap(NULL, sizeof(system_sh_state_t),
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED, fd, 0);
    if (system_state == MAP_FAILED) {
        system_state = NULL;
        fprintf(stderr, "Failed to mmap sys state\n");
        ret = shm_unlink(filename);
        if (ret) fprintf(stderr, "Failed to unlink sys state shm file\n");
        return system_state;
    }

    free(filename);

    return system_state;
}

system_sh_state_t *shm_system_state_get(char *buffer_name)
{
    int ret, fd;
    char* filename = NULL;
    system_sh_state_t *system_state = NULL;

    filename = shm_filename_get(buffer_name, SYS_ST_SUFIX);
    if (!filename) {
        return system_state;
    }

    // Open shared memory file: name, oflags, mode
    fd = shm_open(filename, O_RDWR, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        fprintf(stderr, "Failed to open sys state shm file\n");
        return system_state;
    }

    // Create sys state mmap: address, size, protection, flags, fd, offset
    system_state = mmap(NULL, sizeof(system_sh_state_t),
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED, fd, 0);
    if (system_state == MAP_FAILED) {
        system_state = NULL;
        fprintf(stderr, "Failed to mmap sys state\n");
        ret = shm_unlink(filename);
        if (ret) fprintf(stderr, "Failed to unlink sys state shm file\n");
        return system_state;
    }

    free(filename);

    return system_state;
}

circular_buffer_t *shm_cbuffer_set(char *buffer_name, unsigned int size)
{
    int ret, fd;
    char* filename;
    char* mmap_ptr;
    circular_buffer_t *cbuffer = NULL;
    message_t *messages;
    unsigned int file_size;

    filename = shm_filename_get(buffer_name, CBUFFER_SUFIX);
    if (!filename) {
        return cbuffer;
    }

    file_size = sizeof(circular_buffer_t) + size * sizeof(message_t);

    // Open shared memory file: name, oflags, mode
    fd = shm_open(filename, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        fprintf(stderr, "Failed to open cbuffer shm file\n");
        return cbuffer;
    }

    // Size shared memory file
    ret = ftruncate(fd, file_size);
    if (ret) {
        fprintf(stderr, "Failed to truncate cbuffer shm file\n");
        ret = shm_unlink(filename);
        if (ret) fprintf(stderr, "Failed to unlink cbuffer shm file\n");
        return cbuffer;
    }

    // Create sys state mmap: address, size, protection, flags, fd, offset
    mmap_ptr = mmap(NULL, sizeof(file_size),
                    PROT_READ | PROT_WRITE,
                    MAP_SHARED, fd, 0);
    if (mmap_ptr == MAP_FAILED) {
        fprintf(stderr, "Failed to mmap cbuffer\n");
        ret = shm_unlink(filename);
        if (ret) fprintf(stderr, "Failed to unlink cbuffer shm file\n");
        return cbuffer;
    }

    free(filename);

    cbuffer = (circular_buffer_t *)mmap_ptr;
    mmap_ptr += sizeof(circular_buffer_t);
    messages = (message_t *)mmap_ptr;

    circular_buffer_init_with_cbuffer(cbuffer, messages, size);

    return cbuffer;
}

circular_buffer_t *shm_cbuffer_get(char *buffer_name, unsigned int size,
                                   circular_buffer_t *cbuffer_address)
{
    int ret, fd;
    char* filename;
    circular_buffer_t *cbuffer = NULL;
    unsigned int file_size;

    filename = shm_filename_get(buffer_name, CBUFFER_SUFIX);
    if (!filename) {
        return cbuffer;
    }

    file_size = sizeof(circular_buffer_t) + size * sizeof(message_t);

    // Open shared memory file: name, oflags, mode
    fd = shm_open(filename, O_RDWR, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        fprintf(stderr, "Failed to open cbuffer shm file\n");
        return cbuffer;
    }

    // Create sys state mmap: address, size, protection, flags, fd, offset
    cbuffer = mmap(cbuffer_address, sizeof(file_size),
                    PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_FIXED,
                    fd, 0);
    if (cbuffer == MAP_FAILED) {
        fprintf(stderr, "Failed to mmap cbuffer\n");
        cbuffer = NULL;
        ret = shm_unlink(filename);
        if (ret) fprintf(stderr, "Failed to unlink cbuffer shm file\n");
        return cbuffer;
    }

    free(filename);

    return cbuffer;
}

int sys_state_unmap_close(system_sh_state_t* sys_state, char* buffer_name)
{
    char* filename;
    int ret;

    ret = munmap(sys_state, sizeof(system_sh_state_t));
    if (ret) {
        fprintf(stderr, "Failed to unmap sys state shared memory\n");
        return ret;
    }

    filename = shm_filename_get(buffer_name, CBUFFER_SUFIX);
    if (!filename) return EXIT_FAILURE;

    ret = shm_unlink(filename);
    if (ret) {
        fprintf(stderr, "Failed to unlink sys_state shared memory file\n");
        return ret;
    }
    free(filename);

    return EXIT_SUCCESS;
}

int cbuffer_unmap_close(circular_buffer_t* cbuffer, char* buffer_name)
{
    char* filename;
    unsigned int file_size;
    int ret;

    file_size = sizeof(circular_buffer_t) + cbuffer->length * sizeof(message_t);

    ret = munmap(cbuffer, file_size);
    if (ret) {
        fprintf(stderr, "Failed to unmap cbuffer shared memory\n");
        return ret;
    }

    filename = shm_filename_get(buffer_name, SYS_ST_SUFIX);
    if (!filename) return EXIT_FAILURE;

    ret = shm_unlink(filename);
    if (ret) {
        fprintf(stderr, "Failed to unlink cbuffer shared memory file\n");
        return ret;
    }

    free(filename);
    return EXIT_SUCCESS;
}
