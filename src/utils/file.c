#include "file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* file_read_text(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return NULL;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Allocate buffer for file contents (plus null terminator)
    char* buffer = (char*)malloc(file_size + 1);
    if (!buffer) {
        fprintf(stderr, "Failed to allocate memory for file: %s\n", filename);
        fclose(file);
        return NULL;
    }
    
    // Read file contents
    size_t bytes_read = fread(buffer, 1, file_size, file);
    fclose(file);
    
    if (bytes_read != (size_t)file_size) {
        fprintf(stderr, "Failed to read entire file: %s\n", filename);
        free(buffer);
        return NULL;
    }
    
    // Null-terminate the buffer
    buffer[file_size] = '\0';
    
    return buffer;
}

void* file_read_binary(const char* filename, size_t* size) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open binary file: %s\n", filename);
        return NULL;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (size) {
        *size = (size_t)file_size;
    }
    
    // Allocate buffer for file contents
    void* buffer = malloc(file_size);
    if (!buffer) {
        fprintf(stderr, "Failed to allocate memory for binary file: %s\n", filename);
        fclose(file);
        return NULL;
    }
    
    // Read file contents
    size_t bytes_read = fread(buffer, 1, file_size, file);
    fclose(file);
    
    if (bytes_read != (size_t)file_size) {
        fprintf(stderr, "Failed to read entire binary file: %s\n", filename);
        free(buffer);
        return NULL;
    }
    
    return buffer;
} 