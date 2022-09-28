#pragma once

#include <stdint.h>

/**
 * Reads the given fileName into a null-terminated text buffer
 * @param fileName absolute path with file name
 * @param buffer has at minimum allocated bufferSize + 1
 * @param bufferSize size of buffer
 */
void ReadAllText(const char *fileName, char *buffer, uint64_t *bufferSize);

/**
 * @param fileName absolute path with file name
 * @return size of file in bytes
 */
uint64_t GetFileSize(const char* fileName);

//TODO: similar signature as ReadAllText
const char *ReadBytes(const char *fileName, uint64_t *size);
