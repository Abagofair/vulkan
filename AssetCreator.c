#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include <argtable3.h>
#include <cjson/cJSON.h>

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb/stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "external/stb/stb_image_resize.h"

#include "File.h"
#include "AssetStructures.h"
#include "Utilities.h"

#define ABSOLUTE_PATH_SIZE 256

const char *manifestTextureObjectName = "textures";
const char *manifestShadersObjectName = "shaders";

//Manifest structures
struct ManifestTexture
{
	char *name;
	char *path;
	bool generateMipMaps;
};

enum ShaderType
{
	FRAGMENT
};

struct ManifestShader
{
	char *name;
	char *path;
	enum ShaderType type;
};

struct Assets
{
	struct ManifestTexture **textures;
	uint32_t textureCount;

	struct Shader **shaders;
	uint32_t shaderCount;
};

struct ManifestTexture **ReadTextures(cJSON *textureArray, uint32_t *readCount);
void DestroyTextures(struct ManifestTexture **manifestTextures, uint32_t count);

struct AssetTexture **CreateAssetTextures(struct ManifestTexture **manifestTextures, uint32_t manifestTextureCount);
void DestroyAssetTextures(struct AssetTexture **assetTextures, uint32_t count);

struct ManifestShader *ReadShaders(cJSON *shaderArray);

void WriteAssetFile(const struct Assets *assets, const char *fileName);

int main(int argc, char **argv)
{
	struct arg_file *list = arg_file0(NULL, NULL, "<file>", "manifest file");
	struct arg_lit *help = arg_lit0(NULL, "help", "print this help and exit");
	struct arg_end *end = arg_end(20);
	void *argtable[] = { list, help, end };
	const char *progname = "AssetCreator v0.0.1";
	int nerrors;
	int exitcode = 0;

	/* verify the argtable[] entries were allocated sucessfully */
	if (arg_nullcheck(argtable) != 0)
	{
		/* NULL entries were detected, some allocations must have failed */
		printf("%s: insufficient memory\n", progname);
		exitcode = 1;
		goto exit;
	}

	/* Parse the command line as defined by argtable[] */
	nerrors = arg_parse(argc, argv, argtable);

	/* special case: '--help' takes precedence over error reporting */
	if (help->count > 0)
	{
		printf("Usage: %s", progname);
		arg_print_syntax(stdout, argtable, "\n");
		arg_print_glossary(stdout, argtable, "  %-25s %s\n");
		exitcode = 0;
		goto exit;
	}

	/* If the parser returned any errors then display them and exit */
	if (nerrors > 0)
	{
		/* Display the error details contained in the arg_end struct.*/
		arg_print_errors(stdout, end, progname);
		printf("Try '%s --help' for more information.\n", progname);
		exitcode = 1;
		goto exit;
	}

	/* special case: uname with no command line options induces brief help */
	if (argc == 1)
	{
		printf("Try '%s --help' for more information.\n", progname);
		exitcode = 0;
		goto exit;
	}

	if (list->count != 1)
	{
		exitcode = 1;
		printf("Expected 1 file input\n");
		goto exit;
	}

	char absoluteManifestPath[ABSOLUTE_PATH_SIZE];
	_getcwd(absoluteManifestPath, sizeof absoluteManifestPath);
	errno_t err = strcat_s(absoluteManifestPath, sizeof absoluteManifestPath, list->filename[0]);
	if (err != 0)
	{
		abort();
	}

	uint64_t bufferSize = GetFileSize(absoluteManifestPath);

	char *fileData = malloc(bufferSize * sizeof(char) + 1);
	if (fileData == NULL)
	{
		fprintf(stderr, "Could not allocate for manifest file data\n");
		abort();
	}

	ReadAllText(absoluteManifestPath, fileData, &bufferSize);

	cJSON *manifest = cJSON_Parse(fileData);
	if (manifest == NULL)
	{
		const char *errorPtr = cJSON_GetErrorPtr();
		if (errorPtr != NULL)
		{
			fprintf(stderr, "Error: %s\n", errorPtr);
			goto exit;
		}
	}

	struct Assets assets;

	cJSON *textures = cJSON_GetObjectItemCaseSensitive(manifest, manifestTextureObjectName);
	fprintf(stdout, "Reading manifest textures\n");
	assets.textures = ReadTextures(textures, &assets.textureCount);

	WriteAssetFile(&assets, "test.ass");

exit:
	/* deallocate each non-null entry in argtable[] */
	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
	cJSON_Delete(manifest);

	DestroyTextures(assets.textures, assets.textureCount);
	return exitcode;
}

struct ManifestTexture **ReadTextures(cJSON *textureArray, uint32_t *readCount)
{
	uint32_t textureCount = cJSON_GetArraySize(textureArray);
	if (textureCount <= 0)
	{
		fprintf(stdout, "Did not find any textures in the \"textures\" object\n");
		return NULL;
	}

	struct ManifestTexture **manifestTextures = malloc(textureCount * sizeof(struct ManifestTexture*));
	if (manifestTextures == NULL)
	{
		fprintf(stderr, "Could not allocate struct ManifestTexture *manifestTextures\n");
		abort();
	}

	cJSON *texture = NULL;
	*readCount = 0;
	cJSON_ArrayForEach(texture, textureArray)
	{
		struct ManifestTexture *manifestTexture = malloc(sizeof(struct ManifestTexture));

		cJSON *textureNameItem = cJSON_GetObjectItem(texture, "name");
		if (cJSON_IsString(textureNameItem))
		{
			manifestTexture->name = textureNameItem->valuestring;
		}

		cJSON *texturePathItem = cJSON_GetObjectItem(texture, "path");
		if (cJSON_IsString(texturePathItem))
		{
			manifestTexture->path = texturePathItem->valuestring;
		}

		cJSON *textureMipmapItem = cJSON_GetObjectItem(texture, "generateMipmaps");
		manifestTexture->generateMipMaps = false;
		if (cJSON_IsBool(textureMipmapItem))
		{
			manifestTexture->generateMipMaps = textureMipmapItem->valueint;
		}

		manifestTextures[(*readCount)++] = manifestTexture;
	}

	fprintf(stdout, "Read %i manifest texture objects\n", *readCount);

	return manifestTextures;
}

struct AssetTexture **CreateAssetTextures(struct ManifestTexture **manifestTextures, uint32_t manifestTextureCount)
{
	if (manifestTextureCount <= 0)
	{
		fprintf(stdout, "No manifest textures, skipping creating asset textures\n");
		return NULL;
	}

	struct AssetTexture **assetTextures = malloc(manifestTextureCount * sizeof(struct AssetTexture*));
	if (assetTextures == NULL)
	{
		fprintf(stderr, "Could not allocate struct AssetTexture *s_AssetTextures\n");
		abort();
	}

	for (int i = 0; i < manifestTextureCount; ++i)
	{
		struct ManifestTexture *manifestTexture = manifestTextures[i];
		struct AssetTexture *assetTexture = malloc(sizeof(struct AssetTexture));
		unsigned char* stbiBuffer = stbi_load(manifestTexture->path, &assetTexture->width, &assetTexture->height, &assetTexture->channels, STBI_rgb_alpha);
		assetTexture->channels = 4;
		assetTexture->name = manifestTexture->name;
		assetTexture->mipmap = manifestTexture->generateMipMaps;
		assetTexture->bufferSize = assetTexture->channels * assetTexture->width * assetTexture->height;
		assetTexture->mipmapCount = 0;

		if (assetTexture->width != assetTexture->height || !IsPowerOfTwo(assetTexture->width))
		{
			fprintf(stderr, "Only images with equal width and height that is a power of 2 is currently supported\n");
			stbi_image_free(assetTexture->buffer);
			//todo: free asset and manifest textures
			abort();
		}

		if (assetTexture->mipmap)
		{
			assetTexture->mipmapCount = (uint32_t)log2(assetTexture->width);
			uint32_t maxMipmapBufferSize = (assetTexture->bufferSize / 3) + 1;
			uint32_t bufferSize = maxMipmapBufferSize + assetTexture->bufferSize;
			unsigned char* buffer = malloc(bufferSize);
			int32_t w = assetTexture->width;
			int32_t h = assetTexture->height;
			int32_t w2 = w / 2;
			int32_t h2 = h / 2;

			uint32_t prevSize = 0;

			memcpy(buffer, stbiBuffer, assetTexture->bufferSize);
			stbi_image_free(stbiBuffer);

			for (uint32_t j = 0; j < assetTexture->mipmapCount; ++j)
			{
				uint32_t resizedImageSize = w2 * h2 * assetTexture->channels;
				stbir_resize_uint8(
					&buffer[prevSize],
					w,
					h,
					0,
					&buffer[assetTexture->bufferSize],
					w2,
					h2,
					0,
					assetTexture->channels);

				prevSize = assetTexture->bufferSize;
				assetTexture->bufferSize += resizedImageSize;

				w = w2;
				h = h2;
				w2 /= 2;
				h2 /= 2;
			}

			assetTexture->buffer = malloc(assetTexture->bufferSize);
			memcpy(assetTexture->buffer, buffer, assetTexture->bufferSize);
			free(buffer);
		}
		else
		{
			assetTexture->buffer = malloc(assetTexture->bufferSize);
			memcpy(assetTexture->buffer, stbiBuffer, assetTexture->bufferSize);
			stbi_image_free(stbiBuffer);
		}

		if (assetTexture->buffer == NULL)
		{
			fprintf(stderr, "Could not read ManifestTexture %s at path: %s\n", manifestTexture->name, manifestTexture->path);
			fprintf(stderr, "stbi_failure_reason: %s\n", stbi_failure_reason());
			abort();
		}

		assetTextures[i] = assetTexture;
	}

	return assetTextures;
}

void DestroyTextures(struct ManifestTexture **manifestTextures, uint32_t count)
{
	for (int i = 0; i < count; ++i)
	{
		struct ManifestTexture *manifestTexture = manifestTextures[i];
		free(manifestTexture);
	}
	free(manifestTextures);
}

void DestroyAssetTextures(struct AssetTexture **assetTextures, uint32_t count)
{
	for (int i = 0; i < count; ++i)
	{
		struct AssetTexture *assetTexture = assetTextures[i];
		stbi_image_free(assetTexture->buffer);
		free(assetTexture);
	}
	free(assetTextures);
}

void WriteAssetFile(const struct Assets *assets, const char *fileName)
{
	assert(assets != NULL);

	fprintf(stdout, "Creating asset textures from the read manifest textures\n");
	struct AssetTexture **assetTextures = CreateAssetTextures(assets->textures, assets->textureCount);

	FILE *assetFile = fopen(fileName, "wb");
	errno_t assetFileErr = ferror(assetFile);
	if (assetFileErr != 0)
	{
		fprintf(stderr, "Could not open %s\n", fileName);
		abort();
	}

	fwrite(&assets->textureCount, sizeof(uint32_t), 1, assetFile);
	for (int i = 0; i < assets->textureCount; ++i)
	{
		struct AssetTexture *assetTexture = assetTextures[i];
		uint64_t len = strlen(assetTexture->name);
		fprintf(stdout, "name length %llu\n", len);
		fwrite(&len, sizeof(uint64_t), 1, assetFile);
		fwrite(assetTexture->name, sizeof(char), strlen(assetTexture->name), assetFile);
		fwrite(&assetTexture->width, sizeof(uint32_t), 1, assetFile);
		fwrite(&assetTexture->height, sizeof(uint32_t), 1, assetFile);
		fwrite(&assetTexture->channels, sizeof(uint32_t), 1, assetFile);
		fwrite(&assetTexture->mipmap, sizeof(uint32_t), 1, assetFile);
		fwrite(&assetTexture->mipmapCount, sizeof(uint32_t), 1, assetFile);
		fwrite(&assetTexture->bufferSize, sizeof(uint64_t), 1, assetFile);
		fwrite(assetTexture->buffer, sizeof(unsigned char), assetTexture->bufferSize * sizeof(unsigned char), assetFile);
	}

	fclose(assetFile);

	DestroyAssetTextures(assetTextures, assets->textureCount);
}