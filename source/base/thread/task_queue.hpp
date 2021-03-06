#ifndef SU_BASE_THREAD_TASK_QUEUE_HPP
#define SU_BASE_THREAD_TASK_QUEUE_HPP

#include <atomic>

namespace thread {

template <typename T>
class Task_queue {
  public:
    Task_queue(uint32_t num_tasks);

    ~Task_queue();

    uint32_t size() const;

    void clear();

    void push(T const& task);

    bool pop(T& task);

  public:
    uint32_t const num_tasks_;

    uint32_t top_;

    T* tasks_;

    std::atomic<uint32_t> current_consume_;
};

}  // namespace thread

#endif
