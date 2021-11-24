#include <atomic>
#include <functional>

#define INCOMPLETE 0
#define RUNNING 1
#define COMPLETE 2

namespace skyline::utils {
    class Once {
        public:
            void call_once(std::function<void()> func);

        private:
            std::atomic<uint64_t> inner_state = { INCOMPLETE };

            bool is_complete();
            void block_until_complete();
    };
}
