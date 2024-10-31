#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define SHM_NAME "/my_shared_memory"
#define DOWNLOAD_FILE "good.c"

int main() {
    // Open shared memory object
    int shm_fd = shm_open(SHM_NAME, O_RDONLY, 0666);
    if (shm_fd == -1) {
        perror("Error opening shared memory");
        return 1;
    }

    // Get the size of the shared memory
    struct stat shm_stat;
    if (fstat(shm_fd, &shm_stat) == -1) {
        perror("Error getting shared memory size");
        close(shm_fd);
        return 1;
    }

    size_t shm_size = shm_stat.st_size;

    // Map the shared memory object into the process's address space
    void* shm_ptr = mmap(0, shm_size, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("Error mapping shared memory");
        close(shm_fd);
        return 1;
    }

    printf("Binary program is now accessible from shared memory.\n");

    // Open or create the "downloaded" file for writing
    int download_fd = open(DOWNLOAD_FILE, O_CREAT | O_WRONLY | O_TRUNC, 0777);
    if (download_fd == -1) {
        perror("Error opening/creating the downloaded file");
        munmap(shm_ptr, shm_size);
        close(shm_fd);
        return 1;
    }

    // Write the shared memory contents to the "downloaded" file
    ssize_t bytes_written = write(download_fd, shm_ptr, shm_size);
    if (bytes_written == -1) {
        perror("Error writing to the downloaded file");
        munmap(shm_ptr, shm_size);
        close(download_fd);
        close(shm_fd);
        return 1;
    } else if (bytes_written != shm_size) {
        fprintf(stderr, "Warning: Only %ld out of %ld bytes were written\n", bytes_written, shm_size);
    }

    printf("Binary data successfully written to '%s'.\n", DOWNLOAD_FILE);

    // Close the file descriptor for the downloaded file
    close(download_fd);

    // Clean up shared memory mapping and file descriptors
    munmap(shm_ptr, shm_size);
    close(shm_fd);

    return 0;
}
