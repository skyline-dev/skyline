
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

extern "C" void *__dso_handle = 0;
 
extern "C" int __cxa_atexit(void (*destructor) (void *), void *arg, void *dso) {};
extern "C" void __cxa_finalize(void *f) {};

#ifdef __cplusplus
}
#endif