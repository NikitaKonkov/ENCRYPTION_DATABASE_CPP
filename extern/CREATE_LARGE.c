#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define FILE_NAME "LARGE.txt"
#define FILE_SIZE_GB 100  // Size of the file to create in GB
#define CHUNK_SIZE_MB 512  // Increased for high-performance hardware

int main() {
    FILE *file = fopen(FILE_NAME, "wb");  // Binary mode for faster writing
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    // Set a large buffer for maximum I/O performance
    setvbuf(file, NULL, _IOFBF, 64 * 1024 * 1024);  // 64MB buffer

    // Calculate the total size in bytes and chunk size in bytes
    const uint64_t total_size = (uint64_t)FILE_SIZE_GB * 1024ULL * 1024ULL * 1024ULL;
    const size_t chunk_size = CHUNK_SIZE_MB * 1024 * 1024;

    // Allocate a buffer for the chunk
    char *buffer = (char *)malloc(chunk_size);
    if (buffer == NULL) {
        perror("Error allocating memory");
        fclose(file);
        return 1;
    }

    // Fill the buffer with numbers (e.g., "0123456789")
    for (size_t i = 0; i < chunk_size; i++) {
        buffer[i] = '0' + (i % 10);
    }

    uint64_t written = 0;
    uint64_t last_reported = 0;
    while (written < total_size) {
        size_t to_write = chunk_size;
        if (written + chunk_size > total_size) {
            to_write = total_size - written;
        }

        size_t written_now = fwrite(buffer, 1, to_write, file);
        if (written_now != to_write) {
            perror("Error writing to file");
            free(buffer);
            fclose(file);
            return 1;
        }

        written += written_now;
        
        // Update progress every 10GB to reduce overhead
        if (written - last_reported >= 10ULL * 1024ULL * 1024ULL * 1024ULL || written >= total_size) {
            printf("Progress: %.2f%% (%.2f GB / %d GB)\r", 
                   (double)written / total_size * 100,
                   (double)written / (1024.0 * 1024.0 * 1024.0),
                   FILE_SIZE_GB);
            fflush(stdout);
            last_reported = written;
        }
    }

    printf("\nFile creation complete.\n");

    free(buffer);
    fclose(file);
    return 0;
}