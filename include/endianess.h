/**
 * @file endianess.h
 * @brief Functions for reversing data in between the two endianesses.
 */

#pragma once

#include "types.h"

u16 swap16(u16 x) { return (x << 8) | (x >> 8); }

u32 swap32(u32 x) { return (x << 24) | ((x << 8) & 0xFF0000) | ((x >> 24) & 0xFF) | ((x >> 8) & 0xFF00); }