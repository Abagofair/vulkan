//
// Created by Anders on 13/09/2022.
//

#pragma once

#include <stdint.h>
#include <stdbool.h>

inline
uint32_t ClampU32(
        uint32_t value,
        uint32_t min,
        uint32_t max)
{
    const uint32_t t = value < min ? min : value;
    return t > max ? max : t;
}

inline
bool IsPowerOfTwo(unsigned long x)
{
	return (x != 0) && ((x & (x - 1)) == 0);
}