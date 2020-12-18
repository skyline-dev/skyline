#include "nvn/pfnc.h"

NVN_DEFPROC(nvnDeviceBuilderGetFlags);
NVN_DEFPROC(nvnDeviceBuilderSetDefaults);
NVN_DEFPROC(nvnDeviceBuilderSetFlags);

NVN_DEFPROC(nvnDeviceInitialize);
NVN_DEFPROC(nvnDeviceFinalize);
NVN_DEFPROC(nvnDeviceGetInteger);
NVN_DEFPROC(nvnDeviceGetImageHandle);
NVN_DEFPROC(nvnDeviceGetDepthMode);
NVN_DEFPROC(nvnDeviceGetProcAddress);

NVN_DEFPROC(nvnSyncInitalize);
NVN_DEFPROC(nvnSyncFinalize);
NVN_DEFPROC(nvnSyncWait);
NVN_DEFPROC(nvnSyncSetDebugLabel);

NVN_DEFPROC(nvnWindowInitialize);
NVN_DEFPROC(nvnWindowFinalize);
NVN_DEFPROC(nvnWindowSetCrop);
NVN_DEFPROC(nvnWindowPresentInterval);
NVN_DEFPROC(nvnWindowSetPresentInterval);
NVN_DEFPROC(nvnWindowSetDebugLabel);

NVN_DEFPROC(nvnQueueInitalize);
NVN_DEFPROC(nvnQueueFinalize);
NVN_DEFPROC(nvnQueueFlush);
NVN_DEFPROC(nvnQueueFenceSync);
NVN_DEFPROC(nvnQueueSubmitCommands);
NVN_DEFPROC(nvnQueueWaitSync);
NVN_DEFPROC(nvnQueuePresentTexure);
NVN_DEFPROC(nvnQueueGetError);
NVN_DEFPROC(nvnQueueGetTotalCommandMemoryUsed);
NVN_DEFPROC(nvnQueueGetTotalComputeMemoryUsed);
NVN_DEFPROC(nvnQueueGetTotalControlMemoryUsed);
NVN_DEFPROC(nvnQueueSetDebugLabel);

void nvnInit(NVNdevice* device) {
    nvnDeviceGetProcAddress =
        reinterpret_cast<PFNC_nvnDeviceGetProcAddress>(nvnBootstrapLoader("nvnDeviceGetProcAddress"));

    NVN_GETPROCADDR(nvnDeviceBuilderGetFlags);
    NVN_GETPROCADDR(nvnDeviceBuilderSetDefaults);
    NVN_GETPROCADDR(nvnDeviceBuilderSetFlags);

    NVN_GETPROCADDR(nvnDeviceInitialize);
    NVN_GETPROCADDR(nvnDeviceFinalize);
    NVN_GETPROCADDR(nvnDeviceGetInteger);
    NVN_GETPROCADDR(nvnDeviceGetImageHandle);
    NVN_GETPROCADDR(nvnDeviceGetDepthMode);

    NVN_GETPROCADDR(nvnSyncInitalize);
    NVN_GETPROCADDR(nvnSyncFinalize);
    NVN_GETPROCADDR(nvnSyncWait);
    NVN_GETPROCADDR(nvnSyncSetDebugLabel);

    NVN_GETPROCADDR(nvnWindowInitialize);
    NVN_GETPROCADDR(nvnWindowFinalize);
    NVN_GETPROCADDR(nvnWindowSetCrop);
    NVN_GETPROCADDR(nvnWindowPresentInterval);
    NVN_GETPROCADDR(nvnWindowSetPresentInterval);
    NVN_GETPROCADDR(nvnWindowSetDebugLabel);

    NVN_GETPROCADDR(nvnQueueInitalize);
    NVN_GETPROCADDR(nvnQueueFinalize);
    NVN_GETPROCADDR(nvnQueueFlush);
    NVN_GETPROCADDR(nvnQueueFenceSync);
    NVN_GETPROCADDR(nvnQueueSubmitCommands);
    NVN_GETPROCADDR(nvnQueueWaitSync);
    NVN_GETPROCADDR(nvnQueueGetError);
    NVN_GETPROCADDR(nvnQueueGetTotalCommandMemoryUsed);
    NVN_GETPROCADDR(nvnQueueGetTotalComputeMemoryUsed);
    NVN_GETPROCADDR(nvnQueueGetTotalControlMemoryUsed);
    NVN_GETPROCADDR(nvnQueuePresentTexure);
}