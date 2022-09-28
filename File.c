//
// Created by Anders on 13/09/2022.
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <assert.h>

#include "File.h"

static void GetBinaryFileSize(FILE *file, size_t *size)
{
	fseek(file, 0, SEEK_END);
	*size = ftell(file);
	rewind(file);
}

static FILE *OpenFile(const char *fileName, const char *mode)
{
	FILE *file;
	fopen_s(&file, fileName, mode);

	if (file == NULL)
	{
		fprintf(stderr, "Could not read input file: %s\n", fileName);
		abort();
	}

	return file;
}

uint64_t GetFileSize(const char* fileName)
{
	struct stat sb;
	stat(fileName, &sb);
	return sb.st_size;
}

void ReadAllText(const char *fileName, char *buffer, uint64_t *bufferSize)
{
	assert(fileName != NULL);
	assert(bufferSize != NULL);

	FILE *file = OpenFile(fileName, "r");
	if (file == NULL)
	{
		fprintf(stderr, "Could not open file: %s\n", fileName);
		abort();
	}

	*bufferSize = GetFileSize(fileName) * sizeof(char);

	uint64_t bytesRead = fread_s(buffer, *bufferSize, 1, *bufferSize, file);

	uint32_t hadFileReadError = ferror(file);
	if (hadFileReadError != 0)
	{
		fprintf(stderr, "Error when reading file: %s\n", fileName);
		fclose(file);
		abort();
	}

	if (bytesRead > *bufferSize)
	{
		fprintf(stderr, "Read more bytes than was allocated: %s\n", fileName);
		fclose(file);
		abort();
	}

	//have to null-terminate
	buffer[bytesRead] = '\0';

	fclose(file);
}

//let user allocate void* buffer
const char *ReadBytes(const char *fileName, size_t *size)
{
	FILE *file;
	fopen_s(&file, fileName, "rb");

	if (file == NULL)
	{
		abort();
	}

	fseek(file, 0, SEEK_END);
	*size = ftell(file);
	rewind(file);

	char *fileData = malloc(*size);
	if (fileData == NULL)
	{
		abort();
	}

	size_t bytesRead = fread_s(fileData, *size, 1, *size, file);

	if (bytesRead != *size)
	{
		abort();
	}

	fclose(file);

	return fileData;
}
