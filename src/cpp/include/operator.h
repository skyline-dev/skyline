/**
 * @file operator.h
 * @brief Common operators, and implementations with Heaps.
 */

#pragma once

#include <cstddef>
#include <new>

#include "types.h"

// Nintendo didn't implement these for some reason
void* operator new(std::size_t size, void*);
void* operator new[](std::size_t size, void*);

void* operator new(std::size_t size);
void* operator new[](std::size_t size);
void* operator new(std::size_t size, std::nothrow_t const&);
void* operator new[](std::size_t size, std::nothrow_t const&);
void* operator new(std::size_t size, ulong);
void* operator new[](std::size_t size, ulong);

void operator delete(void*);
void operator delete(void*, std::size_t);
void operator delete(void*, std::nothrow_t const&);
void operator delete[](void*);
void operator delete[](void*, std::size_t);
void operator delete[](void*, std::nothrow_t const&);