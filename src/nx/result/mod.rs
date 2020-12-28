#[derive(Eq, PartialEq, Debug, Copy, Clone)]
#[repr(transparent)]
pub struct NxResult(u32);

const fn mask(bits: u32) -> u32 {
    (1 << bits) - 1
}

impl NxResult {

    const MODULE_BITS: u32 = 9;
    const DESCRIPTION_BITS: u32 = 13;

    const MODULE_MASK: u32 = mask(Self::MODULE_BITS);
    const DESCRIPTION_MASK: u32 = mask(Self::DESCRIPTION_BITS);

    pub const fn new(module: u32, description: u32) -> NxResult {
        NxResult(((module & Self::MODULE_MASK) << Self::DESCRIPTION_BITS) | (description & Self::DESCRIPTION_MASK))
    }

    pub const fn module(self) -> u32 {
        (self.0 >> Self::DESCRIPTION_BITS) & Self::MODULE_MASK
    }

    pub const fn description(self) -> u32 {
        self.0 & Self::DESCRIPTION_MASK
    }
}

impl From<NxResult> for u32 {
    fn from(res: NxResult) -> Self {
        res.into()
    }
}

impl From<u32> for NxResult {
    fn from(num: u32) -> Self {
        NxResult(num)
    }
}

#[macro_export]
macro_rules! make_result {
    ($name:ident, $module:expr, $description:expr) => {
        #[allow(dead_code)]
        pub const $name: NxResult = NxResult::new($module, $description);
    }
}

#[macro_export]
macro_rules! mod_result {
    ($name:ident, $description:expr) => {
        make_result!($name, MODULE, $description);
    };
}

make_result!(SUCCESS, 0, 0);

pub mod fs;
pub mod kern;