//
// Created by Anders on 02/10/2022.
//

#include "AssetManager.h"

#include <stdlib.h>
#include <string.h>

static struct AssetTexture **s_AssetTextures = NULL;
static uint32_t s_AssetTextureCount = 0;

struct AssetTexture *GetTexture(const char* name)
{
	for (uint32_t i = 0; i < s_AssetTextureCount; ++i)
	{
		struct AssetTexture *texture = s_AssetTextures[i];
		if (strcmp(texture->name, name) == 0)
			return texture;
	}

	return NULL;
}

void LoadTextures(FILE *assetFile)
{
	if (s_AssetTextureCount > 0 || s_AssetTextures != NULL)
	{
		fprintf(stderr, "Ensure the currently loaded textures have been destroyed\n");
		return;
	}

	uint32_t textureCount;
	fread(&textureCount, sizeof(uint32_t), 1, assetFile);

	s_AssetTextureCount = 0;
	s_AssetTextures = malloc(textureCount * sizeof(struct AssetTexture*));

	while (s_AssetTextureCount < textureCount)
	{
		struct AssetTexture *assetTexture = malloc(sizeof(struct AssetTexture));

		uint64_t nameLen;
		fread(&nameLen, sizeof(uint64_t), 1, assetFile);
		assetTexture->name = malloc(nameLen * sizeof(char) + 1);
		if (assetTexture->name == NULL)
		{
			fprintf(stderr, "Could not allocate assetTexture->name\n");
			free(assetTexture);
			continue;
		}

		fread(assetTexture->name, sizeof(char), nameLen, assetFile);
		assetTexture->name[nameLen] = '\0';

		fread(&assetTexture->width, sizeof(uint32_t), 1, assetFile);
		fread(&assetTexture->height, sizeof(uint32_t), 1, assetFile);
		fread(&assetTexture->channels, sizeof(uint32_t), 1, assetFile);
		fread(&assetTexture->mipmap, sizeof(uint32_t), 1, assetFile);
		fread(&assetTexture->mipmapCount, sizeof(uint32_t), 1, assetFile);
		fread(&assetTexture->bufferSize, sizeof(uint64_t), 1, assetFile);

		assetTexture->buffer = malloc(assetTexture->bufferSize * sizeof(unsigned char));
		if (assetTexture->buffer == NULL)
		{
			fprintf(stderr, "Could not allocate assetTexture->buffer\n");
			free(assetTexture->name);
			free(assetTexture);
			continue;
		}

		fread(assetTexture->buffer, sizeof(unsigned char), assetTexture->bufferSize * sizeof(unsigned char), assetFile);

		s_AssetTextures[s_AssetTextureCount++] = assetTexture;
	}
}

void DestroyTextures()
{
	if (s_AssetTextures != NULL)
	{
		return;
	}

	for (int i = 0; i < s_AssetTextureCount; ++i)
	{
		struct AssetTexture *assetTexture = s_AssetTextures[i];
		free(assetTexture->name);
		free(assetTexture->buffer);
		free(assetTexture);
	}
	free(s_AssetTextures);

	s_AssetTextureCount = 0;
}

void Destroy()
{
	DestroyTextures();
}