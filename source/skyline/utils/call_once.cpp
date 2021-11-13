#include "skyline/utils/call_once.hpp"
#include "nn/os.hpp"

namespace skyline::utils {
    Once::Once() {
        this->inner_state = INCOMPLETE;
    }

    void Once::call_once(std::function<void()> func) {
        if(this->is_complete()) {
            return;
        }

        auto state = this->inner_state.exchange(RUNNING, std::memory_order_release);

        if(state == COMPLETE) {
            this->inner_state.store(COMPLETE, std::memory_order_release);
            return;
        } else if (state == RUNNING) {
            this->block_until_complete();
        } else if (state == INCOMPLETE) {
            func();
            this->inner_state.store(COMPLETE, std::memory_order_release);
        } else {
            *(int*)0 = 0x45;
        }
    }

    void Once::block_until_complete() {
        while(!this->is_complete()) {
            nn::os::YieldThread();
        }
    }

    bool Once::is_complete() {
        return this->inner_state.load(std::memory_order_acquire) == COMPLETE;
    }
}
