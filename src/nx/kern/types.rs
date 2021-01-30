pub const PAGE_SIZE: usize = 0x1000;

#[derive(Eq, PartialEq)]
pub enum MemoryState {
    Free             = 0x00,
    Io               = 0x01,
    Static           = 0x02,
    Code             = 0x03,
    CodeData         = 0x04,
    Normal           = 0x05,
    Shared           = 0x06,
    Alias            = 0x07,
    AliasCode        = 0x08,
    AliasCodeData    = 0x09,
    Ipc              = 0x0A,
    Stack            = 0x0B,
    ThreadLocal      = 0x0C,
    Transfered       = 0x0D,
    SharedTransfered = 0x0E,
    SharedCode       = 0x0F,
    Inaccessible     = 0x10,
    NonSecureIpc     = 0x11,
    NonDeviceIpc     = 0x12,
    Kernel           = 0x13,
    GeneratedCode    = 0x14,
    CodeOut          = 0x15,
}

bitflags! {
    pub struct MemoryPermission : u32 {
        const NONE          = 0;
        const READ          = 1 << 0;
        const WRITE         = 1 << 1;
        const EXECUTE       = 1 << 2;
        const DONT_CARE     = 1 << 28;

        const READ_WRITE    = Self::READ.bits | Self::WRITE.bits;
        const READ_EXECUTE  = Self::READ.bits | Self::EXECUTE.bits;
    }
}

bitflags! {
    pub struct MemoryAttribute : u32 {
        const LOCKED        = 1 << 0;
        const IPC_LOCKED    = 1 << 1;
        const DEVICE_SHARED = 1 << 2;
        const UNCACHED      = 1 << 3;
    }
}

#[repr(C)]
pub struct PageInfo {
    flags: u32,
}
#[repr(transparent)]
#[derive(Clone, Copy, PartialEq, Eq)]
pub struct Handle(pub u32);

impl Handle {
    pub const INVALID   : Self = Self(0);
    pub const CURR_PROC : Self = Self(0xFFFF8001u32);
}

impl From<Handle> for u32 {
    fn from(v: Handle) -> Self {
        v.0
    }
} 

pub enum MemoryRegionType {
    None              = 0,
    KernelTraceBuffer = 1,
    OnMemoryBootImage = 2,
    DTB               = 3,
}

#[derive(Clone, Copy, PartialEq, Eq)]
pub enum InfoType {
    CoreMask                       = 0,
    PriorityMask                   = 1,
    AliasRegionAddress             = 2,
    AliasRegionSize                = 3,
    HeapRegionAddress              = 4,
    HeapRegionSize                 = 5,
    TotalMemorySize                = 6,
    UsedMemorySize                 = 7,
    DebuggerAttached               = 8,
    ResourceLimit                  = 9,
    IdleTickCount                  = 10,
    RandomEntropy                  = 11,
    AslrRegionAddress              = 12,
    AslrRegionSize                 = 13,
    StackRegionAddress             = 14,
    StackRegionSize                = 15,
    SystemResourceSizeTotal        = 16,
    SystemResourceSizeUsed         = 17,
    ProgramId                      = 18,
    InitialProcessIdRange          = 19,
    UserExceptionContextAddress    = 20,
    TotalNonSystemMemorySize       = 21,
    UsedNonSystemMemorySize        = 22,
    IsApplication                  = 23,
    FreeThreadCount                = 24,

    MesosphereMeta                 = 65000,

    ThreadTickCount                = 0xF0000002,
}

pub enum TickCountInfo {
    Core0 = 0,
    Core1 = 1,
    Core2 = 2,
    Core3 = 3,
}

pub struct MemoryInfo {
    pub base_address: usize,
    pub size: usize,
    pub state: MemoryState,
    pub attribute: MemoryAttribute,
    pub permission: MemoryPermission,
    pub device_refcount: u32,
    pub ipc_refcount: u32,
    pub padding: u32,
}

pub enum CodeMapOperation {
    MapOwner = 0,
    Invalid = -1,
    MapSlave = 1,
    UnmapOwner = 2,
    UnmapSlave = 3,
}