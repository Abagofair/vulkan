//
// Created by Anders on 02/10/2022.
//

#include "AssetManager.h"

#include <stdio.h>
#include <stdlib.h>

static struct AssetTexture **s_AssetTextures = NULL;
static uint32_t s_AssetTextureCount = 0;

void LoadTextures(FILE *assetFile);
static void DestroyTextures();

static void Destroy();

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
		if (assetTexture->name)
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
		fread(&assetTexture->size, sizeof(uint64_t), 1, assetFile);
		fread(&assetTexture->mipmap, sizeof(uint32_t), 1, assetFile);

		assetTexture->buffer = malloc(assetTexture->size * sizeof(unsigned char));
		if (assetTexture->buffer == NULL)
		{
			fprintf(stderr, "Could not allocate assetTexture->name\n");
			free(assetTexture->name);
			free(assetTexture);
			continue;
		}

		fread(assetTexture->buffer, sizeof(unsigned char), assetTexture->size * sizeof(unsigned char), assetFile);

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