#pragma once

#include <stdint.h>
#include <stdbool.h>

struct AssetTexture {
	int32_t width;
	int32_t height;
	int32_t channels;
	int64_t bufferSize;
	bool mipmap;
	uint32_t mipmapCount;
	char *name;
	unsigned char *buffer;
};

struct AssetModel
{
	char *name;
	bool isStatic;
	struct AssetMesh *meshes;
	uint32_t meshCount;
};

struct AssetMesh
{
	char *name;
	bool isStatic;
	uint64_t vertices;
	float *vertexBuffer;
	uint64_t indices;
	uint16_t *indexBuffer;
};