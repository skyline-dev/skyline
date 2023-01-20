#include "skyline/utils/cur_proc_handle.hpp"

#include <cstring>

namespace skyline::proc_handle {
    
    namespace {
        
        Handle s_Handle = INVALID_HANDLE;

        void ReceiveProcessHandleThreadMain(void *session_handle_ptr) {
            // Convert the argument to a handle we can use.
            Handle session_handle = (Handle)(uintptr_t)session_handle_ptr;

            // Receive the request from the client thread.
            memset(armGetTls(), 0, 0x10);
            s32 idx = 0;
            R_ERRORONFAIL(svcReplyAndReceive(&idx, &session_handle, 1, INVALID_HANDLE, UINT64_MAX));

            // Set the process handle.
            s_Handle = ((u32 *)armGetTls())[3];

            // Close the session.
            svcCloseHandle(session_handle);

            // Terminate ourselves.
            svcExitThread();

            // This code will never execute.
            while (true);
        }

        void GetViaIpcTrick(void) {
            alignas(PAGE_SIZE) u8 temp_thread_stack[0x1000];

            // Create a new session to transfer our process handle to ourself
            Handle server_handle, client_handle;
            R_ERRORONFAIL(svcCreateSession(&server_handle, &client_handle, 0, 0));

            // Create a new thread to receive our handle.
            Handle thread_handle;
            R_ERRORONFAIL(svcCreateThread(&thread_handle, (void*) &ReceiveProcessHandleThreadMain, (void *)(uintptr_t)server_handle, temp_thread_stack + sizeof(temp_thread_stack), 0x20, 2));

            // Start the new thread.
            R_ERRORONFAIL(svcStartThread(thread_handle));

            // Send the message.
            static const u32 SendProcessHandleMessage[4] = { 0x00000000, 0x80000000, 0x00000002, CUR_PROCESS_HANDLE };
            memcpy(armGetTls(), SendProcessHandleMessage, sizeof(SendProcessHandleMessage));
            svcSendSyncRequest(client_handle);

            // Close the session handle.
            svcCloseHandle(client_handle);

            // Wait for the thread to be done.
            R_ERRORONFAIL(svcWaitSynchronizationSingle(thread_handle, UINT64_MAX));

            // Close the thread handle.
            svcCloseHandle(thread_handle);
        }

        Result GetViaMesosphere() {
            R_TRY(svcGetInfo((u64*)&s_Handle, InfoType_MesosphereCurrentProcess, INVALID_HANDLE, 0));

            return 0;
        }
    }

    Handle Get() {
        if(s_Handle == INVALID_HANDLE) {
            /* Try to ask mesosphere for our process handle. */
            Result r = GetViaMesosphere();

            /* Fallback to an IPC trick if mesosphere is old/not present. */
            if(R_FAILED(r))
                GetViaIpcTrick();
        }
        return s_Handle;
    }
};