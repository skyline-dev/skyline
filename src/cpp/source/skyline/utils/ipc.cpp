#include "skyline/utils/ipc.hpp"

#include "skyline/utils/cpputils.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#include "skyline/nx/sf/cmif.h"

#ifdef __cplusplus
}
#endif

namespace skyline::utils {

Result nnServiceCreate(Service* srv, const char* name) {
    // open session
    nn::sf::hipc::InitializeHipcServiceResolution();
    nn::svc::Handle svcHandle;
    nn::sf::hipc::ConnectToHipcService(&svcHandle, name);
    nn::sf::hipc::FinalizeHipcServiceResolution();

    void* base = nn::sf::hipc::GetMessageBufferOnTls();

    cmifMakeControlRequest(base, 3, 0);
    R_TRY(nn::sf::hipc::SendSyncRequest(svcHandle, base, 0x100));

    CmifResponse resp = {};
    R_TRY(cmifParseResponse(&resp, base, false, sizeof(u16)));

    // build srv obj
    srv->session = svcHandle.handle;
    srv->own_handle = 1;
    srv->object_id = 0;
    srv->pointer_buffer_size = *(u16*)resp.data;

    return 0;
}

void nnServiceClose(Service* s) {
    void* base = nn::sf::hipc::GetMessageBufferOnTls();

    if (s->own_handle || s->object_id) {
        cmifMakeCloseRequest(base, s->own_handle ? 0 : s->object_id);
        nn::sf::hipc::SendSyncRequest(s->session, base, 0x100);
        if (s->own_handle) nn::sf::hipc::CloseClientSessionHandle(s->session);
    }
    *s = (Service){};
}

Result Ipc::getOwnProcessHandle(Handle* handleOut) {
    Service srv;
    u64 pid;

    svcGetProcessId(&pid, 0xFFFF8001);
    // skyline::utils::writeFile("sd:/tmp/pid.bin", 0, &pid, sizeof(pid));

    nnServiceCreate(&srv, "pm:dmnt");

    Handle tmp_handle;

    struct {
        u64 loc;
        u8 status;
    } out;

    Result rc = nnServiceDispatchInOut(&srv, 65000, pid, out, .out_handle_attrs = {SfOutHandleAttr_HipcCopy},
                                       .out_handles = &tmp_handle, );

    nnServiceClose(&srv);

    if (R_SUCCEEDED(rc)) {
        if (handleOut) {
            *handleOut = tmp_handle;
        } else {
            svcCloseHandle(tmp_handle);
        }
    }

    return 0;
}
}  // namespace skyline::utils
