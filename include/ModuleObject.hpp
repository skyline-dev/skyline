#pragma once

#include <assert.h>
#include <elf.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

namespace rtld {
struct ModuleObject {
   private:
    // ResolveSymbols internals
    inline void ResolveSymbolRelAbsolute(Elf64_Rel* entry);
    inline void ResolveSymbolRelaAbsolute(Elf64_Rela* entry);
    inline void ResolveSymbolRelJumpSlot(Elf64_Rel* entry, bool do_lazy_got_init);
    inline void ResolveSymbolRelaJumpSlot(Elf64_Rela* entry, bool do_lazy_got_init);

   public:
    struct ModuleObject* next;
    struct ModuleObject* prev;
    union {
        Elf64_Rel* rel;
        Elf64_Rela* rela;
        void* raw;
    } rela_or_rel_plt;
    union {
        Elf64_Rel* rel;
        Elf64_Rela* rela;
    } rela_or_rel;
    uint64_t module_base;
    Elf64_Dyn* dynamic;
    bool is_rela;
    uint64_t rela_or_rel_plt_size;
    void (*dt_init)(void);
    void (*dt_fini)(void);
    uint32_t* hash_bucket;
    uint32_t* hash_chain;
    char* dynstr;
    Elf64_Sym* dynsym;
    uint64_t dynstr_size;
    void** got;
    uint64_t rela_dyn_size;
    uint64_t rel_dyn_size;
    uint64_t rel_count;
    uint64_t rela_count;
    uint64_t hash_nchain_value;
    uint64_t hash_nbucket_value;
    uint64_t got_stub_ptr;
#ifdef __RTLD_6XX__
    uint64_t soname_idx;
    uint64_t nro_size;
    bool cannot_revert_symbols;
#endif

    void Initialize(uint64_t aslr_base, Elf64_Dyn* dynamic);
    void Relocate();
    Elf64_Sym* GetSymbolByName(const char* name);
    void ResolveSymbols(bool do_lazy_got_init);
    bool TryResolveSymbol(Elf64_Addr* target_symbol_address, Elf64_Sym* symbol);
};

#ifdef __RTLD_6XX__
static_assert(sizeof(ModuleObject) == 0xD0, "ModuleObject size isn't valid");
#else
static_assert(sizeof(ModuleObject) == 0xB8, "ModuleObject size isn't valid");
#endif
}  // namespace rtld