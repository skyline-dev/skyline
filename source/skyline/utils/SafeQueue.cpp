#include "skyline/utils/SafeQueue.hpp"

namespace skyline {
namespace Utils {

    Task::Task(){
        nn::os::InitializeEvent(&completionEvent, false, true);
    }

    Task::~Task(){
        nn::os::FinalizeEvent(&completionEvent);
    }

    void entrypoint(void* a){
        SafeTaskQueue* queue = (SafeTaskQueue*) a;
        queue->_threadEntrypoint();
    }

    void SafeTaskQueue::startThread(s32 priority, s32 core, u64 stackSize) {
        void* stack = malloc(stackSize);
        nn::os::CreateThread(&thread, entrypoint, this, stack, stackSize, priority, core);
    }

    void SafeTaskQueue::_threadEntrypoint(){
        while(true){
            std::shared_ptr<Task*> task;
            if(pop(&task, nn::TimeSpan::FromNanoSeconds(10000000))){
                (*task)->taskFunc();
                nn::os::SignalEvent(&(*task)->completionEvent);
                nn::os::FinalizeEvent(&(*task)->completionEvent);
            }
        }
    }

    template<typename T>
    SafeQueue<T>::SafeQueue(u64 count) {
        buffer = new T[count];
        nn::os::InitializeMessageQueue(&queue, buffer, sizeof(T) * count);
    }

    template<typename T>
    void SafeQueue<T>::push(std::shared_ptr<T*> ptr){
        nn::os::SendMessageQueue(&queue, (u64) ptr);
    }

    template<typename T>
    bool SafeQueue<T>::push(std::shared_ptr<T*> ptr, nn::TimeSpan span){
        return nn::os::TimedSendMessageQueue(&queue, (u64) ptr, span);
    }

    template<typename T>
    void SafeQueue<T>::pop(std::shared_ptr<T*>* ptr){
        nn::os::ReceiveMessageQueue((u64*) ptr, &queue);
    }   
    template<typename T>
    bool SafeQueue<T>::pop(std::shared_ptr<T*>* ptr, nn::TimeSpan span){
        return nn::os::TimedReceiveMessageQueue((u64*) ptr, &queue, span);
    }

    template<typename T>
    bool SafeQueue<T>::tryPush(std::shared_ptr<T*> ptr){
        return nn::os::TrySendMessageQueue(&queue, (u64) ptr);
    }

    template<typename T>
    bool SafeQueue<T>::tryPop(std::shared_ptr<T*>* ptr){
        return nn::os::TryReceiveMessageQueue((u64*) ptr, &queue);
    }

    template<typename T>
    SafeQueue<T>::~SafeQueue(){
        nn::os::FinalizeMessageQueue(&queue);
        delete[] buffer;
    }
};
};