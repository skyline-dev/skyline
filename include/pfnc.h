/**
 * @file pfnc.h
 * @brief Functions that communicate with the NVN function pool.
 */

#pragma once

// there's a lot of these
static void* pfnc_nvnMemoryPoolGetFlags;
static void* pfnc_nvnMemoryPoolFlushMappedRange;
static void* pfnc_nvnTexturePoolInitialize;
static void* pfnc_nvnTexturePoolSetDebugLabel;
static void* pfnc_nvnTexturePoolFinalize;