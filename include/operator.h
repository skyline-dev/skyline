/**
 * @file operator.h
 * @brief Common operators, and implementations with Heaps.
 */

#pragma once

#include <cstddef>
#include <std.h>
#include "types.h"

// Nintendo didn't implement these for some reason
void* WEAK operator new(std::size_t size, void*);
void* WEAK operator new[](std::size_t size, void*);

void* WEAK operator new(std::size_t size);
void* WEAK operator new[](std::size_t size);
void* WEAK operator new(std::size_t size, std::nothrow_t const &);
void* WEAK operator new[](std::size_t size, std::nothrow_t const &);
void* WEAK operator new(std::size_t size, ulong);
void* WEAK operator new[](std::size_t size, ulong);

void WEAK operator delete(void *);
void WEAK operator delete(void *, std::size_t);
void WEAK operator delete(void *, std::nothrow_t const &);
void WEAK operator delete[](void *);
void WEAK operator delete[](void *, std::size_t);
void WEAK operator delete[](void *, std::nothrow_t const &);