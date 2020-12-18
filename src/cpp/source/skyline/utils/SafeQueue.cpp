#include "skyline/utils/SafeQueue.hpp"

namespace skyline::utils {

Task::Task() { nn::os::InitializeEvent(&completionEvent, false, nn::os::EventClearMode_AutoClear); }

Task::Task(std::function<void()> taskFunc) : Task() { this->taskFunc = taskFunc; }

Task::~Task() { nn::os::FinalizeEvent(&completionEvent); }

void entrypoint(void* a) {
    SafeTaskQueue* queue = (SafeTaskQueue*)a;
    queue->_threadEntrypoint();
}

SafeTaskQueue::SafeTaskQueue(u64 count) : SafeQueue::SafeQueue(count) {}

void SafeTaskQueue::startThread(s32 priority, s32 core, u64 stackSize) {
    skyline::logger::s_Instance->Log("[SafeTaskQueue] Starting thread.\n");
    void* stack = memalign(0x1000, stackSize);
    Result rc = nn::os::CreateThread(&thread, entrypoint, this, stack, stackSize, priority, core);
    if (R_FAILED(rc)) {
        skyline::logger::s_Instance->LogFormat("[SafeTaskQueue] Failed to create thread (0x%x).", rc);
    }

    nn::os::StartThread(&thread);
}

void SafeTaskQueue::_threadEntrypoint() {
    skyline::logger::s_Instance->Log("[SafeTaskQueue] Thread started.\n");
    while (true) {
        std::unique_ptr<Task>* taskptr;
        if (pop(&taskptr, nn::TimeSpan::FromNanoSeconds(10000000))) {
            Task* task = taskptr->get();

            // run task
            task->taskFunc();

            // signal that the task is complete
            nn::os::SignalEvent(&task->completionEvent);

            // release task, freeing event
            taskptr->release();

            delete taskptr;
        }
    }
}
};  // namespace skyline::utils
