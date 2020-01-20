/**
 * @file pfnc.h
 * @brief Functions that communicate with the NVN function pool.
 */

#pragma once

#include "nvn/types.h"

#include <string>

#define NVN_DEFPROC(s) PFNC_##s s
#define NVN_GETPROCADDR(s) s = reinterpret_cast<PFNC_ ##s>(nvnDeviceGetProcAddress(device, #s))

void nvnInit(NVNdevice*);

extern "C" void* nvnBootstrapLoader(const char*);

// DEVICE BUILDER
typedef NVNdeviceFlags (*PFNC_nvnDeviceBuilderGetFlags)(NVNdeviceBuilder*);
typedef void (*PFNC_nvnDeviceBuilderSetDefaults)(NVNdeviceBuilder*);
typedef void (*PFNC_nvnDeviceBuilderSetFlags)(NVNdeviceBuilder*, u32);

// DEVICE
typedef bool (*PFNC_nvnDeviceInitialize)(NVNdevice*,  NVNdeviceBuilder*);
typedef void (*PFNC_nvnDeviceFinalize)(NVNdevice*);
typedef void (*PFNC_nvnDeviceGetInteger)(NVNdevice*, NVNdeviceInfo, u32*);
typedef NVNimageHandle (*PFNC_nvnDeviceGetImageHandle)(NVNdevice*, u32);
typedef NVNdepthMode (*PFNC_nvnDeviceGetDepthMode)(NVNdevice*);
typedef void* (*PFNC_nvnDeviceGetProcAddress)(NVNdevice*, const char*);

// SYNC
typedef bool (*PFNC_nvnSyncInitalize)(NVNsync*, NVNdevice*);
typedef void (*PFNC_nvnSyncFinalize)(NVNsync*);
typedef u32 (*PFNC_nvnSyncWait)(NVNsync*, u64);
typedef void (*PFNC_nvnSyncSetDebugLabel)(NVNsync*, char*);

// WINDOW
typedef bool (*PFNC_nvnWindowInitialize)(NVNwindow*, const NVNwindowBuilder*);
typedef void (*PFNC_nvnWindowFinalize)(NVNwindow*);
typedef void (*PFNC_nvnWindowSetCrop)(NVNwindow*, int x, int y, int width, int height);
typedef int  (*PFNC_nvnWindowPresentInterval)(NVNwindow*);
typedef void (*PFNC_nvnWindowSetPresentInterval)(NVNwindow*, int);
typedef void (*PFNC_nvnWindowSetDebugLabel)(NVNwindow*, char*);

// QUEUE
typedef bool (*PFNC_nvnQueueInitalize)(NVNqueue*);
typedef void (*PFNC_nvnQueueFinalize)(NVNqueue*);
typedef void (*PFNC_nvnQueueFlush)(NVNqueue*);
typedef void (*PFNC_nvnQueueFenceSync)(NVNqueue*, NVNsync*, NVNsync*, u32, u32);
typedef void (*PFNC_nvnQueueSubmitCommands)(NVNqueue*, u32, NVNcommandHandle*);
typedef bool (*PFNC_nvnQueueWaitSync)(NVNqueue*, NVNsync*);
typedef void (*PFNC_nvnQueuePresentTexure)(NVNqueue*, NVNwindow*, u32);
typedef u8 (*PFNC_nvnQueueGetError)(NVNqueue*, u64*);
typedef size_t (*PFNC_nvnQueueGetTotalCommandMemoryUsed)(NVNqueue*);
typedef size_t (*PFNC_nvnQueueGetTotalComputeMemoryUsed)(NVNqueue*);
typedef size_t (*PFNC_nvnQueueGetTotalControlMemoryUsed)(NVNqueue*);
typedef void (*PFNC_nvnQueueResetMemoryUsageCounts)(NVNqueue*);
typedef void (*PFNC_nvnQueueSetDebugLabel)(NVNqueue*, char*);

extern NVN_DEFPROC(nvnDeviceBuilderGetFlags);
extern NVN_DEFPROC(nvnDeviceBuilderSetDefaults);
extern NVN_DEFPROC(nvnDeviceBuilderSetFlags);

extern NVN_DEFPROC(nvnDeviceInitialize);
extern NVN_DEFPROC(nvnDeviceFinalize);
extern NVN_DEFPROC(nvnDeviceGetInteger);
extern NVN_DEFPROC(nvnDeviceGetImageHandle);
extern NVN_DEFPROC(nvnDeviceGetDepthMode);
extern NVN_DEFPROC(nvnDeviceGetProcAddress);

extern NVN_DEFPROC(nvnSyncInitalize);
extern NVN_DEFPROC(nvnSyncFinalize);
extern NVN_DEFPROC(nvnSyncWait);
extern NVN_DEFPROC(nvnSyncSetDebugLabel);

extern NVN_DEFPROC(nvnWindowInitialize);
extern NVN_DEFPROC(nvnWindowFinalize);
extern NVN_DEFPROC(nvnWindowSetCrop);
extern NVN_DEFPROC(nvnWindowPresentInterval);
extern NVN_DEFPROC(nvnWindowSetPresentInterval);
extern NVN_DEFPROC(nvnWindowSetDebugLabel);

extern NVN_DEFPROC(nvnQueueInitalize);
extern NVN_DEFPROC(nvnQueueFinalize);
extern NVN_DEFPROC(nvnQueueFlush);
extern NVN_DEFPROC(nvnQueueFenceSync);
extern NVN_DEFPROC(nvnQueueSubmitCommands);
extern NVN_DEFPROC(nvnQueueWaitSync);
extern NVN_DEFPROC(nvnQueuePresentTexure);
extern NVN_DEFPROC(nvnQueueGetError);
extern NVN_DEFPROC(nvnQueueGetTotalCommandMemoryUsed);
extern NVN_DEFPROC(nvnQueueGetTotalComputeMemoryUsed);
extern NVN_DEFPROC(nvnQueueGetTotalControlMemoryUsed);
extern NVN_DEFPROC(nvnQueueSetDebugLabel);