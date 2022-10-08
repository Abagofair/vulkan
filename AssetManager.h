#pragma once

#include <stdio.h>

#include "AssetStructures.h"

struct AssetTexture *GetTexture(const char* name);
void LoadTextures(FILE *assetFile);
void DestroyTextures();
void Destroy();