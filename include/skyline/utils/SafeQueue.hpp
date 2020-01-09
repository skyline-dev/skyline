#pragma once

#include "types.h"

#include "nn/os.h"

#include <queue>
#include <memory>
#include <functional>

namespace skyline {
namespace Utils {

    class Task {
        public:
        
        std::function<void()> taskFunc;
        nn::os::EventType completionEvent;

        Task();
        ~Task();
    };

    template<typename T>
    class SafeQueue {
        private:
        T* buffer;

        protected:
        nn::os::MessageQueueType queue;

        public:
        SafeQueue(u64);
        
        void push(std::shared_ptr<T*>);
        bool push(std::shared_ptr<T*>, nn::TimeSpan);
        void pop(std::shared_ptr<T*>*);
        bool pop(std::shared_ptr<T*>*, nn::TimeSpan);

        bool tryPush(std::shared_ptr<T*>);
        bool tryPop(std::shared_ptr<T*>*);

        ~SafeQueue();
    };


    class SafeTaskQueue : SafeQueue<Task> {
        public:
        nn::os::ThreadType thread;

        void startThread(s32 priority, s32 core, u64 stackSize);
        void _threadEntrypoint();
    };
};
};