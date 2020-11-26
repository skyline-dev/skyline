#include "skyline/logger/TcpLogger.hpp"

#include "skyline/utils/cpputils.hpp"

#define PORT 6969

extern "C" void skyline_tcp_send_raw(char* data, size_t size) __attribute__((visibility("default")));

void skyline_tcp_send_raw(char* data, u64 size) { skyline::logger::s_Instance->Log(data, size); }

namespace skyline::logger {
int g_tcpSocket;
bool g_loggerInit = false;

Result stub(){
    return 0;
};

void TcpLogger::Initialize() {
    const size_t poolSize = 0x600000;
    void* socketPool = memalign(0x4000, poolSize);

    Result (*nnSocketInitalizeImpl)(void*, ulong, ulong, int);

    A64HookFunction(reinterpret_cast<void*>(nn::socket::Initialize), reinterpret_cast<void*>(stub),
                    (void**)&nnSocketInitalizeImpl);  // prevent trying to init sockets twice (crash)

    A64HookFunction(reinterpret_cast<void*>(nn::socket::Finalize), reinterpret_cast<void*>(stub),
                    NULL);  // prevent it being deinit either

    nnSocketInitalizeImpl(socketPool, poolSize, 0x20000, 14);

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

    g_loggerInit = true;
}

void TcpLogger::SendRaw(void* data, size_t size) { nn::socket::Send(g_tcpSocket, data, size, 0); }
};  // namespace skyline::logger
