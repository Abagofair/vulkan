#include <argtable3.h>
#include <cjson/cJSON.h>

#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "File.h"

const char *manifestTextureObjectName = "textures";
const char *manifestShadersObjectName = "shaders";

//Manifest structures
struct Texture
{
	char *name;
	char *path;
	bool generateMipMaps;
};

enum ShaderType
{
	FRAGMENT
};

struct Shader
{
	char *name;
	char *path;
	enum ShaderType type;
};

struct Assets
{
	struct Texture *textures;
	struct Shader *shaders;
};

struct Texture *ReadTextures(cJSON *textureArray);
struct Shader *ReadShaders(cJSON *shaderArray);

/*
 * SECTION NAME
 * ITEM NAME
 * MIPMAP (1 or 0)
 * ITEM LENGTH IN BYTES
 * ..
 * ..
 *
 * TEXTURES\0
 * ASSET_TEXTURE_1\0
 * 1\0
 * 512\0
 * 90eyrgh9serghsergh8serg00d8hgd08hgdfæg09dkfg
 */
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

	/* normal case: take the command line options at face value */
	printf("%d\n", list->count);
	printf("%s\n", list->filename[0]);

	if (list->count != 1)
	{
		exitcode = 1;
		printf("Expected 1 file input\n");
		goto exit;
	}

	char absoluteManifestPath[512];
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

	cJSON *textures = cJSON_GetObjectItemCaseSensitive(manifest, manifestTextureObjectName);
	fprintf(stdout, "%s\n", cJSON_Print(textures));
	cJSON *texture = NULL;
	cJSON_ArrayForEach(texture, textures)
	{
		cJSON *textureNameItem = cJSON_GetObjectItem(texture, "name");
		const char *textureName = cJSON_GetStringValue(textureNameItem);
		fprintf(stdout, "%s\n", textureName);
	}

exit:
	/* deallocate each non-null entry in argtable[] */
	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
	cJSON_Delete(manifest);

	return exitcode;
}