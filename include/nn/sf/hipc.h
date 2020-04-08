#pragma once

#include "types.h"

#include "../svc.h"

namespace nn::sf::hipc {
    void* GetMessageBufferOnTls();

    Result InitializeHipcServiceResolution();
    Result ConnectToHipcService(nn::svc::Handle *,char const*);
    Result FinalizeHipcServiceResolution();

    Result SendSyncRequest(nn::svc::Handle,void *,ulong);
    Result CloseClientSessionHandle(nn::svc::Handle);

    namespace detail {
        
    }
};