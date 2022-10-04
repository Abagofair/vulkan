#pragma once

#include <stdint.h>
#include <stdbool.h>

struct AssetTexture {
	int32_t width;
	int32_t height;
	int32_t channels;
	int64_t size;
	bool mipmap;
	char *name;
	unsigned char *buffer;
};