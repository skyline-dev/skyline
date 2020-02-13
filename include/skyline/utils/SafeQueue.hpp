#pragma once

#include "types.h"

#include "mem.h"
#include "nn/os.h"
#include "skyline/logger/TcpLogger.hpp"

#include <queue>
#include <memory>
#include <functional>

namespace skyline {
namespace utils {

    class Task {
        public:
        
        std::function<void()> taskFunc;
        nn::os::EventType completionEvent;

        Task();
        Task(std::function<void()> taskFunc);

        ~Task();
    };

    template<typename T>
    class SafeQueue {
        private:
        T** buffer;

        protected:
        nn::os::MessageQueueType queue;

        public:
        SafeQueue(u64 count) {
            buffer = (T**) malloc(sizeof(T*) * count);
            nn::os::InitializeMessageQueue(&queue, (u64*) buffer, sizeof(T*) * count);
        }

        void push(std::unique_ptr<T>* ptr){
            nn::os::SendMessageQueue(&queue, *reinterpret_cast<u64*>(&ptr));
        }
        bool push(std::unique_ptr<T>* ptr, nn::TimeSpan span){
            return nn::os::TimedSendMessageQueue(&queue, (u64) ptr, span);
        }

        void pop(std::unique_ptr<T>** ptr){
            nn::os::ReceiveMessageQueue((u64*) ptr, &queue);
        }
        bool pop(std::unique_ptr<T>** ptr, nn::TimeSpan span){
            return nn::os::TimedReceiveMessageQueue((u64*) ptr, &queue, span);
        }

        bool tryPush(std::unique_ptr<T>* ptr){
            return nn::os::TrySendMessageQueue(&queue, (u64) ptr);
        }
        bool tryPop(std::unique_ptr<T>** ptr){
            return nn::os::TryReceiveMessageQueue((u64*) ptr, &queue);
        }

        ~SafeQueue(){
            nn::os::FinalizeMessageQueue(&queue);
            delete[] buffer;
        }
    };


    class SafeTaskQueue : public SafeQueue<Task> {
        public:
        nn::os::ThreadType thread;
        
        SafeTaskQueue(u64);

        void startThread(s32 priority, s32 core, u64 stackSize);
        void _threadEntrypoint();
    };
};
};