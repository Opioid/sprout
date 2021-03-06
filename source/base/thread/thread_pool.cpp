#include "thread_pool.hpp"

#ifdef GRANULAR_TASKS
#include "task_queue.inl"
#endif

#include <algorithm>
#include <cmath>

namespace thread {

Pool::Pool(uint32_t num_threads)
    : num_threads_(num_threads),
      uniques_(new Unique[num_threads]),
      threads_(new std::thread[num_threads])
#ifdef GRANULAR_TASKS
      ,
      tasks_(num_threads * 64)
#endif
{
    for (uint32_t i = 0; i < num_threads; ++i) {
        threads_[i] = std::thread(&Pool::loop, this, i);
    }

    async_thread_ = std::thread(async_loop, std::ref(async_));
}

Pool::~Pool() {
    quit_ = true;

    wake_all(num_threads_);

    for (uint32_t i = 0, len = num_threads_; i < len; ++i) {
        threads_[i].join();
    }

    async_.quit = true;

    wake_async();

    async_thread_.join();

    delete[] threads_;
    delete[] uniques_;
}

uint32_t Pool::num_threads() const {
    return num_threads_;
}

bool Pool::is_running_parallel() const {
    return running_parallel_;
}

void Pool::run_parallel(Parallel_program&& program, uint32_t num_tasks_hint) {
    parallel_program_ = program;

    range_program_ = nullptr;

    uint32_t const num_tasks = num_tasks_hint ? std::min(num_tasks_hint, num_threads_)
                                              : num_threads_;

    wake_all(num_tasks);

    wait_all(num_tasks);
}

void Pool::run_range(Range_program&& program, int32_t begin, int32_t end) {
    range_program_ = program;

    parallel_program_ = nullptr;

    wake_all(begin, end);

    wait_all(num_threads_);
}

void Pool::run_async(Async_program&& program) {
    wait_async();

    async_.program = std::move(program);

    wake_async();
}

void Pool::wait_async() {
    std::unique_lock<std::mutex> lock(async_.mutex);
    async_.done_signal.wait(lock, [this]() { return !async_.wake; });
}

uint32_t Pool::num_threads(int32_t request) {
    uint32_t const available_threads = std::max(std::thread::hardware_concurrency(), 1u);

    if (request <= 0) {
        int32_t const num_threads = static_cast<int32_t>(available_threads) + request;

        return uint32_t(std::max(num_threads, 1));
    }

    return std::min(available_threads, uint32_t(std::max(request, 1)));
}

void Pool::wake_all(uint32_t num_tasks) {
    running_parallel_ = true;

    for (uint32_t i = 0; i < num_tasks; ++i) {
        auto& u = uniques_[i];

        std::unique_lock<std::mutex> lock(u.mutex);
        u.wake = true;
        lock.unlock();
        u.wake_signal.notify_one();
    }
}

void Pool::wake_all(int32_t begin, int32_t end) {
    running_parallel_ = true;

#ifdef GRANULAR_TASKS
    float const range     = float(end - begin);
    float const num_tasks = float(tasks_.size());

    int32_t const step = int32_t(std::ceil(range / num_tasks));

    tasks_.clear();

    for (int32_t i = begin; i < end; i += step) {
        tasks_.push({i, std::min(i + step, end)});
    }

    wake_all();
#else
    float const range       = float(end - begin);
    float const num_threads = float(num_threads_);

    int32_t const step = int32_t(std::ceil(range / num_threads));

    int32_t b = 0;
    int32_t e = begin;

    for (uint32_t i = 0, len = num_threads_; i < len; ++i) {
        auto& u = uniques_[i];

        b = e;
        e += step;

        std::unique_lock<std::mutex> lock(u.mutex);
        u.begin = b;
        u.end   = std::min(e, end);
        u.wake  = true;
        lock.unlock();
        u.wake_signal.notify_one();
    }
#endif
}

void Pool::wake_async() {
    std::unique_lock<std::mutex> lock(async_.mutex);
    async_.wake = true;
    lock.unlock();
    async_.wake_signal.notify_one();
}

void Pool::wait_all(uint32_t num_tasks) {
    for (uint32_t i = 0; i < num_tasks; ++i) {
        auto& u = uniques_[i];

        std::unique_lock<std::mutex> lock(u.mutex);
        u.done_signal.wait(lock, [&u]() { return !u.wake; });
    }

    running_parallel_ = false;
}

void Pool::loop(uint32_t id) {
    Unique& unique = uniques_[id];

    for (;;) {
        std::unique_lock<std::mutex> lock(unique.mutex);
        unique.wake_signal.wait(lock, [&unique]() { return unique.wake; });

        if (quit_) {
            break;
        }

        if (range_program_) {
#ifdef GRANULAR_TASKS
            for (Task task; tasks_.pop(task);) {
                range_program_(id, task.begin, task.end);
            }
#else
            range_program_(id, unique.begin, unique.end);
#endif
        } else if (parallel_program_) {
            parallel_program_(id);
        }

        unique.wake = false;
        lock.unlock();
        unique.done_signal.notify_one();
    }
}

void Pool::async_loop(Async& async) {
    for (;;) {
        std::unique_lock<std::mutex> lock(async.mutex);
        async.wake_signal.wait(lock, [&async]() { return async.wake; });

        if (async.quit) {
            break;
        }

        async.program();

        async.wake = false;
        lock.unlock();
        async.done_signal.notify_one();
    }
}

}  // namespace thread
