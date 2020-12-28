use crate::nx::result::NxResult;

const MODULE: u32 = 1;

mod_result!(PATH_NOT_FOUND,         1);
mod_result!(PATH_ALREADY_EXISTS,    2);
mod_result!(TARGET_LOCKED,          7);
mod_result!(DIRECTORY_NOT_EMPTY,    8);