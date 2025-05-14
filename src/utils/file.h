#ifndef FILE_H
#define FILE_H

#include <stddef.h>

/**
 * @brief Reads the entire contents of a text file into a C string
 * 
 * @param filename The path to the file to read
 * @return char* A pointer to the file contents, or NULL if the file could not be read.
 *               The caller is responsible for freeing this memory.
 */
char* file_read_text(const char* filename);

/**
 * @brief Reads the entire contents of a binary file
 * 
 * @param filename The path to the file to read
 * @param size Pointer to a size_t where the file size will be stored
 * @return void* A pointer to the file contents, or NULL if the file could not be read.
 *               The caller is responsible for freeing this memory.
 */
void* file_read_binary(const char* filename, size_t* size);

#endif /* FILE_H */ 