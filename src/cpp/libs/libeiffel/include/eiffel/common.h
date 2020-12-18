#pragma once

#include <stddef.h>
#include <stdint.h>

#define SL_PLUGIN_NAME_SIZE 16
typedef char SlPluginName[SL_PLUGIN_NAME_SIZE];

// subject to change
typedef struct {
    SlPluginName name;
    uint32_t version;
    uint32_t apiVersion;
} SlPluginMeta;

typedef struct {
    uint32_t handle;
    size_t size;
    uint32_t perm;
} SlPluginSharedMemInfo;
