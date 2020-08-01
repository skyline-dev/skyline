#pragma once

#include "nn/sf/hipc.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "skyline/nx/sf/service.h"

#ifdef __cplusplus
}
#endif

namespace skyline::utils {

Result nnServiceCreate(Service* srv, const char* name);
void nnServiceClose(Service* s);

NX_INLINE void* nnServiceMakeRequest(Service* s, u32 request_id, u32 context, u32 data_size, bool send_pid,
                                     const SfBufferAttrs buffer_attrs, const SfBuffer* buffers, u32 num_objects,
                                     const Service* const* objects, u32 num_handles, const Handle* handles) {
    void* base = nn::sf::hipc::GetMessageBufferOnTls();

    CmifRequestFormat fmt = {};
    fmt.object_id = s->object_id;
    fmt.request_id = request_id;
    fmt.context = context;
    fmt.data_size = data_size;
    fmt.server_pointer_size = s->pointer_buffer_size;
    fmt.num_objects = num_objects;
    fmt.num_handles = num_handles;
    fmt.send_pid = send_pid;

    _serviceRequestFormatProcessBuffer(&fmt, buffer_attrs.attr0);
    _serviceRequestFormatProcessBuffer(&fmt, buffer_attrs.attr1);
    _serviceRequestFormatProcessBuffer(&fmt, buffer_attrs.attr2);
    _serviceRequestFormatProcessBuffer(&fmt, buffer_attrs.attr3);
    _serviceRequestFormatProcessBuffer(&fmt, buffer_attrs.attr4);
    _serviceRequestFormatProcessBuffer(&fmt, buffer_attrs.attr5);
    _serviceRequestFormatProcessBuffer(&fmt, buffer_attrs.attr6);
    _serviceRequestFormatProcessBuffer(&fmt, buffer_attrs.attr7);

    CmifRequest req = cmifMakeRequest(base, fmt);

    if (s->object_id)  // TODO: Check behavior of input objects in non-domain sessions
        for (u32 i = 0; i < num_objects; i++) cmifRequestObject(&req, objects[i]->object_id);

    for (u32 i = 0; i < num_handles; i++) cmifRequestHandle(&req, handles[i]);

    _serviceRequestProcessBuffer(&req, &buffers[0], buffer_attrs.attr0);
    _serviceRequestProcessBuffer(&req, &buffers[1], buffer_attrs.attr1);
    _serviceRequestProcessBuffer(&req, &buffers[2], buffer_attrs.attr2);
    _serviceRequestProcessBuffer(&req, &buffers[3], buffer_attrs.attr3);
    _serviceRequestProcessBuffer(&req, &buffers[4], buffer_attrs.attr4);
    _serviceRequestProcessBuffer(&req, &buffers[5], buffer_attrs.attr5);
    _serviceRequestProcessBuffer(&req, &buffers[6], buffer_attrs.attr6);
    _serviceRequestProcessBuffer(&req, &buffers[7], buffer_attrs.attr7);

    return req.data;
}

NX_INLINE Result nnServiceParseResponse(Service* s, u32 out_size, void** out_data, u32 num_out_objects,
                                        Service* out_objects, const SfOutHandleAttrs out_handle_attrs,
                                        Handle* out_handles) {
    void* base = nn::sf::hipc::GetMessageBufferOnTls();

    CmifResponse res = {};
    bool is_domain = s->object_id != 0;
    Result rc = cmifParseResponse(&res, base, is_domain, out_size);
    if (R_FAILED(rc)) return rc;

    if (out_size) *out_data = res.data;

    for (u32 i = 0; i < num_out_objects; i++) {
        if (is_domain)
            serviceCreateDomainSubservice(&out_objects[i], s, cmifResponseGetObject(&res));
        else  // Output objects are marshalled as move handles at the beginning of the list.
            serviceCreateNonDomainSubservice(&out_objects[i], s, cmifResponseGetMoveHandle(&res));
    }

    _serviceResponseGetHandle(&res, out_handle_attrs.attr0, &out_handles[0]);
    _serviceResponseGetHandle(&res, out_handle_attrs.attr1, &out_handles[1]);
    _serviceResponseGetHandle(&res, out_handle_attrs.attr2, &out_handles[2]);
    _serviceResponseGetHandle(&res, out_handle_attrs.attr3, &out_handles[3]);
    _serviceResponseGetHandle(&res, out_handle_attrs.attr4, &out_handles[4]);
    _serviceResponseGetHandle(&res, out_handle_attrs.attr5, &out_handles[5]);
    _serviceResponseGetHandle(&res, out_handle_attrs.attr6, &out_handles[6]);
    _serviceResponseGetHandle(&res, out_handle_attrs.attr7, &out_handles[7]);

    return 0;
}

NX_INLINE Result nnServiceDispatchImpl(Service* s, u32 request_id, const void* in_data, u32 in_data_size,
                                       void* out_data, u32 out_data_size, SfDispatchParams disp) {
    void* base = nn::sf::hipc::GetMessageBufferOnTls();
    Service srv = *s;

    void* in =
        nnServiceMakeRequest(&srv, request_id, disp.context, in_data_size, disp.in_send_pid, disp.buffer_attrs,
                             disp.buffers, disp.in_num_objects, disp.in_objects, disp.in_num_handles, disp.in_handles);

    if (in_data_size) __builtin_memcpy(in, in_data, in_data_size);

    R_TRY(nn::sf::hipc::SendSyncRequest(disp.target_session == INVALID_HANDLE ? s->session : disp.target_session, base,
                                        0x100));

    void* out = NULL;
    R_TRY(nnServiceParseResponse(&srv, out_data_size, &out, disp.out_num_objects, disp.out_objects,
                                 disp.out_handle_attrs, disp.out_handles));

    if (out_data && out_data_size) __builtin_memcpy(out_data, out, out_data_size);

    return 0;
}

#define nnServiceDispatch(_s, _rid, ...) \
    skyline::utils::ServiceDispatchImpl((_s), (_rid), NULL, 0, NULL, 0, (SfDispatchParams){__VA_ARGS__})

#define nnServiceDispatchIn(_s, _rid, _in, ...) \
    skyline::utils::nnServiceDispatchImpl((_s), (_rid), &(_in), sizeof(_in), NULL, 0, (SfDispatchParams){__VA_ARGS__})

#define nnServiceDispatchOut(_s, _rid, _out, ...)                                       \
    skyline::utils::nnServiceDispatchImpl((_s), (_rid), NULL, 0, &(_out), sizeof(_out), \
                                          (SfDispatchParams){__VA_ARGS__})

#define nnServiceDispatchInOut(_s, _rid, _in, _out, ...)                                            \
    skyline::utils::nnServiceDispatchImpl((_s), (_rid), &(_in), sizeof(_in), &(_out), sizeof(_out), \
                                          (SfDispatchParams){__VA_ARGS__})

class Ipc {
   public:
    static Result getOwnProcessHandle(Handle*);
};

}  // namespace skyline::utils
