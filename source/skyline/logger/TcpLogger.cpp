#include "skyline/logger/TcpLogger.hpp"

#include "skyline/utils/cpputils.hpp"

#define PORT 6969

extern "C" void skyline_tcp_send_raw(char* data, size_t size) __attribute__((visibility("default")));

void skyline_tcp_send_raw(char* data, u64 size) { skyline::logger::s_Instance->Log(data, size); }

namespace skyline::logger {
int g_tcpSocket = -1;
bool g_loggerInit = false;

Result stub(){
    return 0;
};

Result (*nnSocketInitalizeImpl)(void*, ulong, ulong, int);
Result (*nnSocketInitalizeConfigImpl)(nn::socket::Config const&);

void init_socket_thing() {
    struct sockaddr_in serverAddr;
    g_tcpSocket = nn::socket::Socket(AF_INET, SOCK_STREAM, 0);
    if (g_tcpSocket & 0x80000000) return;

    int flags = 1;
    nn::socket::SetSockOpt(g_tcpSocket, SOL_SOCKET, SO_KEEPALIVE, &flags, sizeof(flags));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = nn::socket::InetHtons(PORT);

    int rval = nn::socket::Bind(g_tcpSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (rval < 0) return;

    rval = nn::socket::Listen(g_tcpSocket, 1);
    if (rval < 0) return;

    u32 addrLen;
    g_tcpSocket = nn::socket::Accept(g_tcpSocket, (struct sockaddr*)&serverAddr, &addrLen);
    
    char* message = "TCP Socket Connnected.\n";
    nn::socket::Send(g_tcpSocket, (void*)message, strlen(message), 0);
}

void start_logger_init_thread() {
    const size_t stackSize = 0x3000;
    void* threadStack = memalign(0x1000, stackSize);

    nn::os::ThreadType* thread = new nn::os::ThreadType;
    nn::os::CreateThread(thread, init_socket_thing, nullptr, threadStack, stackSize, 16, 0);
    nn::os::StartThread(thread);
}

Result init_normal(void* arg1, ulong arg2, ulong arg3, int arg4) {
    if (g_loggerInit == false) {
        g_loggerInit = true;

        const size_t poolSize = 0x600000;
        void* socketPool = memalign(0x4000, poolSize);
        Result res = nnSocketInitalizeImpl(socketPool, poolSize, 0x20000, 14);

        start_logger_init_thread();

        return res;
    }
}

Result init_config(nn::socket::Config const& config) {
    if (g_loggerInit == false) {
        g_loggerInit = true;

        const size_t poolSize = 0x600000;
        void* socketPool = memalign(0x4000, poolSize);
        Result res = nnSocketInitalizeImpl(socketPool, poolSize, 0x20000, 14);

        start_logger_init_thread();

        return res;
    }
}

void setup_socket_hooks() {
    Result (*socketInitWithPool)(void*, ulong, ulong, int) = nn::socket::Initialize;
    A64HookFunction(reinterpret_cast<void*>(socketInitWithPool), reinterpret_cast<void*>(init_normal),
                    (void**)&nnSocketInitalizeImpl);  // prevent trying to init sockets twice (crash)

    Result (*socketInitWithConfig)(nn::socket::Config const&) = nn::socket::Initialize;
    A64HookFunction(reinterpret_cast<void*>(socketInitWithConfig), reinterpret_cast<void*>(init_config), (void**)&nnSocketInitalizeConfigImpl);

    A64HookFunction(reinterpret_cast<void*>(nn::socket::Finalize), reinterpret_cast<void*>(stub),
                    NULL);  // prevent it being deinit either
}

void TcpLogger::Initialize() {}

bool TcpLogger::ShouldFlush() {
    return g_loggerInit && (g_tcpSocket != -1);
}

void TcpLogger::SendRaw(void* data, size_t size) {
    nn::socket::Send(g_tcpSocket, data, size, 0);
}
};  // namespace skyline::logger
