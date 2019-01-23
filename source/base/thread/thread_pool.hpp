#ifndef SU_BASE_THREAD_POOL_HPP
#define SU_BASE_THREAD_POOL_HPP

#include <condition_variable>
#include <functional>
#include <thread>

// #define GRANULAR_TASKS

#ifdef GRANULAR_TASKS
#include "task_queue.hpp"
#endif

namespace thread {

class Pool {
  public:
    using Parallel_program = std::function<void(uint32_t)>;
    using Range_program    = std::function<void(uint32_t, int32_t, int32_t)>;
    using Async_program    = std::function<void()>;

    Pool(uint32_t num_threads) noexcept;

    ~Pool() noexcept;

    uint32_t num_threads() const noexcept;

    void run_parallel(Parallel_program program) noexcept;

    void run_range(Range_program program, int32_t begin, int32_t end) noexcept;

    void run_async(Async_program program) noexcept;

    void wait_async() noexcept;

  private:
    void wake_all() noexcept;
    void wake_all(int32_t begin, int32_t end) noexcept;

    void wake_async() noexcept;

    void wait_all() noexcept;

    struct Unique {
        int32_t                 begin;
        int32_t                 end;
        std::condition_variable wake_signal;
        std::condition_variable done_signal;
        std::mutex              mutex;
        bool                    wake = false;
    };

    struct Async {
        Async_program           program;
        std::condition_variable wake_signal;
        std::condition_variable done_signal;
        std::mutex              mutex;
        bool                    wake = false;
        bool                    quit = false;
    };

    uint32_t num_threads_;

    bool quit_ = false;

    Parallel_program parallel_program_;

    Range_program range_program_;

    Unique* uniques_;

    std::thread* threads_;

    struct Task {
        int32_t begin;
        int32_t end;
    };

#ifdef GRANULAR_TASKS
    Task_queue<Task> tasks_;
#endif

    Async async_;

    std::thread async_thread_;

    void loop(uint32_t id) noexcept;

    static void async_loop(Async& async) noexcept;
};

}  // namespace thread

#endif
