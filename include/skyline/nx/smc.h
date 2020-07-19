/**
 * @file smc.h
 * @brief Wrappers for secure monitor calls.
 * @copyright libnx Authors
 */
#pragma once

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    EXO_EMUMMC_TYPE_NONE = 0,
    EXO_EMUMMC_TYPE_PARTITION = 1,
    EXO_EMUMMC_TYPE_FILES = 2,
} exo_emummc_type_t;

typedef enum {
    EXO_EMUMMC_MMC_NAND = 0,
    EXO_EMUMMC_MMC_SD = 1,
    EXO_EMUMMC_MMC_GC = 2,
} exo_emummc_mmc_t;

typedef struct {
    uint32_t magic;
    uint32_t type;
    uint32_t id;
    uint32_t fs_version;
} exo_emummc_base_config_t;

typedef struct {
    uint64_t start_sector;
} exo_emummc_partition_config_t;

typedef struct {
    exo_emummc_base_config_t base_cfg;
    union {
        exo_emummc_partition_config_t partition_cfg;
    };
} exo_emummc_config_t;

void smcRebootToRcm(void);
void smcRebootToIramPayload(void);
void smcPerformShutdown(void);

Result smcCopyToIram(uintptr_t iram_addr, const void* src_addr, u32 size);
Result smcCopyFromIram(void* dst_addr, uintptr_t iram_addr, u32 size);

Result smcReadWriteRegister(u32 phys_addr, u32 value, u32 mask);

void* smcMemCpy(void* dst_addr, void* src_addr, size_t size);
void* smcMemSet(void* dst_addr, u32 value, size_t size);

Result smcWriteAddress8(void* dst_addr, u8 val);
Result smcWriteAddress16(void* dst_addr, u16 val);
Result smcWriteAddress32(void* dst_addr, u32 val);
Result smcWriteAddress64(void* dst_addr, u64 val);

Result smcGetEmummcConfig(exo_emummc_mmc_t mmc_id, exo_emummc_config_t* out_cfg, void* out_paths);

#ifdef __cplusplus
}
#endif