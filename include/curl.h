/**
 * @file curl.h
 * @brief CURL implementation.
 */

#pragma once

#include "types.h"

typedef u32 CURLCode;  // basic code for getting results from functions

#ifdef __cplusplus
extern "C" {
#endif

typedef void CURL;
enum CURLINFO {};
enum CURLoption {};

// GLOBAL
CURLcode curl_global_init(s64 flags);

// EASY
CURL* curl_easy_init();
CURLcode curl_easy_setopt(CURL* curl, CURLoption, ...);
CURLcode curl_easy_perform(CURL* curl);
void curl_easy_cleanup(CURL* curl);
CURLcode curl_easy_getinfo(CURL* curl, CURLINFO info, ...);
CURL* curl_easy_duphandle(CURL* curl);
void curl_easy_reset(CURL* curl);
CURLcode curl_easy_pause(CURL* curl, s32 mask);
CURLcode curl_easy_recv(CURL* curl, void* buffer, u64 bufferLength, u64*);
CURLcode curl_easy_send(CURL* curl, void const* buffer, u64 bufferLength, u64*);

#ifdef __cplusplus
}
#endif