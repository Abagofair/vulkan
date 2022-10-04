#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <argtable3.h>
#include <cjson/cJSON.h>

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "external/stb/stb_image_write.h"

#include "File.h"
#include "AssetStructures.h"

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
	struct ManifestTexture *textures;
	uint32_t textureCount;

	struct Shader *shaders;
	uint32_t shaderCount;
};

struct ManifestTexture *ReadTextures(cJSON *textureArray, uint32_t *readCount);
void DestroyTextures(struct ManifestTexture *manifestTextures, uint32_t count);

struct AssetTexture *CreateAssetTextures(struct ManifestTexture *manifestTextures, uint32_t manifestTextureCount);
void DestroyAssetTextures(struct AssetTexture *assetTextures, uint32_t count);

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

struct ManifestTexture *ReadTextures(cJSON *textureArray, uint32_t *readCount)
{
	uint32_t textureCount = cJSON_GetArraySize(textureArray);
	if (textureCount <= 0)
	{
		fprintf(stdout, "Did not find any textures in the \"textures\" object\n");
		return NULL;
	}

	struct ManifestTexture *manifestTextures = malloc(textureCount * sizeof(struct ManifestTexture));
	if (manifestTextures == NULL)
	{
		fprintf(stderr, "Could not allocate struct ManifestTexture *manifestTextures\n");
		abort();
	}

	cJSON *texture = NULL;
	*readCount = 0;
	cJSON_ArrayForEach(texture, textureArray)
	{
		struct ManifestTexture manifestTexture;

		cJSON *textureNameItem = cJSON_GetObjectItem(texture, "name");
		if (cJSON_IsString(textureNameItem))
		{
			manifestTexture.name = textureNameItem->valuestring;
		}

		cJSON *texturePathItem = cJSON_GetObjectItem(texture, "path");
		if (cJSON_IsString(texturePathItem))
		{
			manifestTexture.path = texturePathItem->valuestring;
		}

		cJSON *textureMipmapItem = cJSON_GetObjectItem(texture, "generateMipmaps");
		manifestTexture.generateMipMaps = false;
		if (cJSON_IsBool(textureMipmapItem))
		{
			manifestTexture.generateMipMaps = textureMipmapItem->valueint;
		}

		manifestTextures[(*readCount)++] = manifestTexture;
	}

	fprintf(stdout, "Read %i manifest texture objects\n", *readCount);

	return manifestTextures;
}

struct AssetTexture *CreateAssetTextures(struct ManifestTexture *manifestTextures, uint32_t manifestTextureCount)
{
	if (manifestTextureCount <= 0)
	{
		fprintf(stdout, "No manifest textures, skipping creating asset textures\n");
		return NULL;
	}

	struct AssetTexture *assetTextures = malloc(manifestTextureCount * sizeof(struct AssetTexture));
	if (assetTextures == NULL)
	{
		fprintf(stderr, "Could not allocate struct AssetTexture *s_AssetTextures\n");
		abort();
	}

	for (int i = 0; i < manifestTextureCount; ++i)
	{
		struct ManifestTexture manifestTexture = manifestTextures[i];
		struct AssetTexture assetTexture;
		assetTexture.buffer = stbi_load(manifestTexture.path, &assetTexture.width, &assetTexture.height, &assetTexture.channels, STBI_rgb_alpha);
		assetTexture.channels = 4;
		if (assetTexture.buffer == NULL)
		{
			fprintf(stderr, "Could not read ManifestTexture %s at path: %s\n", manifestTexture.name, manifestTexture.path);
			fprintf(stderr, "stbi_failure_reason: %s\n", stbi_failure_reason());
			abort();
		}

		assetTexture.name = manifestTexture.name;
		assetTexture.mipmap = manifestTexture.generateMipMaps;
		assetTexture.size = assetTexture.channels * assetTexture.width * assetTexture.height;

		assetTextures[i] = assetTexture;
	}

	return assetTextures;
}

void DestroyTextures(struct ManifestTexture *manifestTextures, uint32_t count)
{
	free(manifestTextures);
}

void DestroyAssetTextures(struct AssetTexture *assetTextures, uint32_t count)
{
	for (int i = 0; i < count; ++i)
	{
		struct AssetTexture assetTexture = assetTextures[i];
		stbi_image_free(assetTexture.buffer);
	}
}

void WriteAssetFile(const struct Assets *assets, const char *fileName)
{
	assert(assets != NULL);

	fprintf(stdout, "Creating asset textures from the read manifest textures\n");
	struct AssetTexture *assetTextures = CreateAssetTextures(assets->textures, assets->textureCount);

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
		struct AssetTexture assetTexture = assetTextures[i];
		uint64_t len = strlen(assetTexture.name);
		fprintf(stdout, "name length %llu\n", len);
		fwrite(&len, sizeof(uint64_t), 1, assetFile);
		fwrite(assetTexture.name, sizeof(char), strlen(assetTexture.name), assetFile);
		fwrite(&assetTexture.width, sizeof(uint32_t), 1, assetFile);
		fwrite(&assetTexture.height, sizeof(uint32_t), 1, assetFile);
		fwrite(&assetTexture.channels, sizeof(uint32_t), 1, assetFile);
		fwrite(&assetTexture.size, sizeof(uint64_t), 1, assetFile);
		fwrite(&assetTexture.mipmap, sizeof(uint32_t), 1, assetFile);
		fwrite(assetTexture.buffer, sizeof(unsigned char), assetTexture.size * sizeof(unsigned char), assetFile);
	}

	fclose(assetFile);

	//test
	FILE *file = fopen(fileName, "rb");

	uint32_t textureCount;
	fread(&textureCount, sizeof(uint32_t), 1, file);

	for (int i = 0; i < textureCount; ++i)
	{
		struct AssetTexture assetTexture;
		uint64_t nameLen;
		fread(&nameLen, sizeof(uint64_t), 1, file);
		assetTexture.name = malloc(nameLen * sizeof(char) + 1);

		fprintf(stdout, "name length %llu\n", nameLen);

		fread(assetTexture.name, sizeof(char), nameLen, assetFile);
		assetTexture.name[nameLen] = '\0';

		fread(&assetTexture.width, sizeof(uint32_t), 1, assetFile);
		fread(&assetTexture.height, sizeof(uint32_t), 1, assetFile);
		fread(&assetTexture.channels, sizeof(uint32_t), 1, assetFile);
		fread(&assetTexture.size, sizeof(uint64_t), 1, assetFile);
		fread(&assetTexture.mipmap, sizeof(uint32_t), 1, assetFile);

		assetTexture.buffer = malloc(assetTexture.size * sizeof(unsigned char));
		fread(assetTexture.buffer, sizeof(unsigned char), assetTexture.size * sizeof(unsigned char), assetFile);
		fprintf(stdout, "name %s\n", assetTexture.name);
		fprintf(stdout, "width %i\n", assetTexture.width);
		fprintf(stdout, "height %i\n", assetTexture.height);
		fprintf(stdout, "channels %i\n", assetTexture.channels);
		fprintf(stdout, "size %llu\n", assetTexture.size);
		fprintf(stdout, "mipmap %i\n", assetTexture.mipmap);

		char fname[256];
		fname[255] = '\0';
		strcat_s(fname, 255, assetTexture.name);
		strcat_s(fname, 4, ".jpg");
		fprintf(stdout, "%s\n", fname);
		//STBIWDEF int stbi_write_jpg(char const *filename, int x, int y, int comp, const void  *data, int quality);
		stbi_write_jpg("from_ass_file.jpg", assetTexture.width, assetTexture.height, assetTexture.channels, assetTexture.buffer, 70);
	}

	DestroyAssetTextures(assetTextures, assets->textureCount);
}