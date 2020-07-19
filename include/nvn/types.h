#pragma once

#include "../types.h"

typedef u64 NVNcommandHandle;
typedef u64 NVNdeviceInfo;
typedef u64 NVNimageHandle;
typedef u64 NVNdepthMode;
typedef u32 NVNdeviceFlags;

typedef struct {
    char _x0[0x3000];
} NVNdevice;

typedef struct {
    char _x0[0x2000];
} NVNqueue;

typedef struct {
    char _x0[0xA0];
} NVNcommandBuffer;

typedef struct {
    char _x0[0x180];
} NVNwindow;

typedef struct {
    char _x0[0x40];
} NVNwindowBuilder;

typedef struct {
    char _x0[0x40];
} NVNsync;

typedef struct {
    char _x0[0x40];
} NVNdeviceBuilder;