//
// Created by Anders on 13/09/2022.
//

#pragma once

const char *ReadBytes(const char *fileName, uint64_t *size);
void WriteBytes(const char *fileName, const uint32_t *bytes, size_t count);
