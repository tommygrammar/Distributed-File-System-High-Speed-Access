#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define SHM_NAME "/my_shared_memory"  // Shared memory object name

int main() {
    const char* file_name = "/home/tommy/Desktop/My projects/Optimized-FIle-System/good.c";  // Name of the compiled binary file
    int shm_fd;  // Shared memory file descriptor
    void* shm_ptr;  // Pointer to the shared memory region

    // Open the binary file for reading
    int file_fd = open(file_name, O_RDONLY);
    if (file_fd == -1) {
        perror("Error opening binary file");
        return 1;
    }

    // Get the size of the binary file
    struct stat file_stat;
    if (fstat(file_fd, &file_stat) == -1) {
        perror("Error getting binary file size");
        close(file_fd);
        return 1;
    }
    size_t file_size = file_stat.st_size;

    // Display the file size
    printf("Binary file size: %ld bytes\n", file_size);

    // Open shared memory object (create if it doesn't exist)
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Error opening shared memory");
        close(file_fd);
        return 1;
    }

    // Set the size of the shared memory object to match the file size
    if (ftruncate(shm_fd, file_size) == -1) {
        perror("Error setting shared memory size");
        close(shm_fd);
        close(file_fd);
        return 1;
    }

    // Map the shared memory object into the process's address space
    shm_ptr = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("Error mapping shared memory");
        close(shm_fd);
        close(file_fd);
        return 1;
    }

    // Read the binary file into the shared memory
    ssize_t bytes_read = read(file_fd, shm_ptr, file_size);
    if (bytes_read == -1) {
        perror("Error reading binary file into shared memory");
        munmap(shm_ptr, file_size);
        close(shm_fd);
        close(file_fd);
        return 1;
    } else if (bytes_read != file_size) {
        fprintf(stderr, "Warning: Only %ld out of %ld bytes were read\n", bytes_read, file_size);
    }

    printf("Binary file '%s' successfully loaded into shared memory\n", file_name);

    // Keep the shared memory object mapped for other processes or unmap if done
    // munmap(shm_ptr, file_size);  // Uncomment if you want to unmap immediately

    // Clean up file descriptors
    close(file_fd);
    close(shm_fd);

    return 0;
}
