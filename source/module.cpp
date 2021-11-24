#include "ModuleObject.hpp"

#define MODULE_NAME "Skyline"
#define MODULE_NAME_LEN 7

// rtld working object
__attribute__((section(".bss"))) rtld::ModuleObject __nx_module_runtime;

struct ModuleName {
    int unknown;
    int name_lengh;
    char name[MODULE_NAME_LEN + 1];
};

__attribute__((section(".rodata.module_name")))
const ModuleName module_name = {.unknown = 0, .name_lengh = MODULE_NAME_LEN, .name = MODULE_NAME};
