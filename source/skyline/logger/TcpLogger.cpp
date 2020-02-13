#include "skyline/logger/TcpLogger.hpp"

#define IP      "10.4.1.157"
#define PORT    6969

namespace skyline {
    int g_tcpSocket;
    bool g_loggerInit = false;

    std::queue<char*>* g_msgQueue = nullptr;

    void TcpLogger::ThreadMain(void*){

        TcpLogger::Initialize();

        TcpLogger::LogFormat("[TcpLogger] Connection established.");

        while(true){
            TcpLogger::ClearQueue();
            nn::os::YieldThread(); // let other parts of OS do their thing
            //nn::os::SleepThread(nn::TimeSpan::FromNanoSeconds(100000000));
        }
    }

    void stub() {};

    // *Must* be ran on core 3
    void TcpLogger::StartThread(){
        const size_t poolSize = 0x100000;
        void* socketPool = memalign(0x4000, poolSize);
        
        nn::socket::Initialize(socketPool, poolSize, 0x20000, 14);

        A64HookFunction(reinterpret_cast<void*>(nn::socket::Initialize), reinterpret_cast<void*>(stub), NULL); // prevent Smash trying to init sockets twice (crash)
        
        const size_t stackSize = 0x3000;
        void* threadStack = memalign(0x1000, stackSize);
        
        nn::os::ThreadType* thread = new nn::os::ThreadType;
        nn::os::CreateThread(thread, ThreadMain, NULL, threadStack, stackSize, 16, 0);
        nn::os::StartThread(thread);
    }

    void TcpLogger::SendRaw(const char* data)
    {
        SendRaw((void*)data, strlen(data));
    }

    void TcpLogger::SendRawFormat(const char* format, ...)
    {
        va_list args;
        char buff[0x1000] = {0};
        va_start(args, format);

        int len = vsnprintf(buff, sizeof(buff), format, args);
        
        TcpLogger::SendRaw(buff, len);
        
        va_end (args);
    }

    void TcpLogger::SendRaw(void* data, size_t size)
    {
        nn::socket::Send(g_tcpSocket, data, size, 0);
    }

    void AddToQueue(char* data)
    {
        if(!g_msgQueue)
            g_msgQueue = new std::queue<char*>();

        g_msgQueue->push(data);
    }

    void TcpLogger::ClearQueue()
    {
        if(!g_msgQueue)
            return;

        while (!g_msgQueue->empty())
        {
            auto data = g_msgQueue->front();

            SendRaw(data, strlen(data));
            delete[] data;
            g_msgQueue->pop();
        }
    }

    void TcpLogger::Initialize()
    {
        if (g_loggerInit)
            return;

        struct sockaddr_in serverAddr;
        g_tcpSocket = nn::socket::Socket(AF_INET, SOCK_STREAM, 0);
        if(g_tcpSocket & 0x80000000)
            return;

        int flags = 1;
        nn::socket::SetSockOpt(g_tcpSocket, SOL_SOCKET, SO_KEEPALIVE, &flags, sizeof(flags));

        serverAddr.sin_family      = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port        = nn::socket::InetHtons(PORT);

        int rval = nn::socket::Bind(g_tcpSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        if (rval < 0)
            return;
        
        rval = nn::socket::Listen(g_tcpSocket, 1);
        if(rval < 0)
            return;

        u32 addrLen;
        g_tcpSocket = nn::socket::Accept(g_tcpSocket, (struct sockaddr*)&serverAddr, &addrLen);  

        g_loggerInit = true;
        
        TcpLogger::ClearQueue();
    }



    void TcpLogger::Log(const char* data, size_t size)
    {
        //Initialize();

        if (size == UINT32_MAX)
            size = strlen(data);

        char* ptr = new char[size+2];
        memset(ptr, 0, size+2);
        memcpy(ptr, data, size);
        ptr[size] = '\n';

        AddToQueue(ptr);
        return;
    }

    void TcpLogger::Log(std::string str)
    {
        TcpLogger::Log(str.data(), str.size());
    }

    void TcpLogger::LogFormat(const char* format, ...)
    {
        va_list args;
        va_start(args, format);

        size_t len = vsnprintf(NULL, 0, format, args);
        char* ptr = new char[len+2];
        memset(ptr, 0, len+2);
        vsnprintf(ptr, len+1, format, args);
        ptr[len] = '\n';
         
        AddToQueue(ptr);
        va_end (args);

        return;

    }
};