//
// Created by Anders on 13/09/2022.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "File.h"

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

void WriteBytes(const char *fileName, const uint32_t *bytes, size_t count)
{
	FILE *file;
	fopen_s(&file, fileName, "wb");

	if (file == NULL)
	{
		abort();
	}

	size_t bytesWritten = fwrite(bytes, 1, count, file);

	if (bytesWritten != count)
	{
		abort();
	}

	fclose(file);
}
