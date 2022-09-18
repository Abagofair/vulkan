//
// Created by Anders on 13/09/2022.
//

#pragma once

#include <stdint.h>

inline
uint32_t ClampU32(
        uint32_t value,
        uint32_t min,
        uint32_t max)
{
    const uint32_t t = value < min ? min : value;
    return t > max ? max : t;
}