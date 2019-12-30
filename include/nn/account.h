/**
 * @file account.h
 * @brief Account service implementation.
 */

#pragma once

#include "os.h"
#include "types.h"

namespace nn
{
    namespace account
    {
        typedef char Nickname[0x21];
        typedef u64 Uid[0x2];
        typedef u64 NetworkServiceAccountId;
        
        class AsyncContext;
        struct UserHandle;

        void Initialize();
        Result ListAllUsers(s32 *, nn::account::Uid *, s32 numUsers);
        Result OpenUser(nn::account::UserHandle *, nn::account::Uid const &);
        Result IsNetworkServiceAccountAvailable(bool *out, nn::account::UserHandle const &);
        void CloseUser(nn::account::UserHandle const &);

        Result EnsureNetworkServiceAccountAvailable(nn::account::UserHandle const &userHandle);
        Result EnsureNetworkServiceAccountIdTokenCacheAsync(nn::account::AsyncContext *, nn::account::UserHandle const &);
        Result LoadNetworkServiceAccountIdTokenCache(u64 *, char *, u64, nn::account::UserHandle const &);

        Result GetLastOpenedUser(nn::account::Uid *);
        Result GetNickname(nn::account::Nickname *nickname, nn::account::Uid const &userID);

        class AsyncContext
        {
        public:
            AsyncContext();

            Result HasDone(bool *);
            Result GetResult();
            Result Cancel();
            Result GetSystemEvent(nn::os::SystemEvent *);
        };
    };
};