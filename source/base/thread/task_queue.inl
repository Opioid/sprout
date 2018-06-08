#ifndef SU_BASE_THREAD_TASK_QUEUE_INL
#define SU_BASE_THREAD_TASK_QUEUE_INL

#include "memory/align.hpp"
#include "task_queue.hpp"

namespace thread {

template <typename T>
Task_queue<T>::Task_queue(uint32_t num_tasks)
    : num_tasks_(num_tasks),
      tasks_(memory::allocate_aligned<T>(num_tasks)),
      current_consume_(num_tasks) {}

template <typename T>
Task_queue<T>::~Task_queue() {
  memory::free_aligned(tasks_);
}

template <typename T>
uint32_t Task_queue<T>::size() const {
  return num_tasks_;
}

template <typename T>
void Task_queue<T>::clear() {
  top_ = 0;
  current_consume_ = 0;
}

template <typename T>
void Task_queue<T>::restart() {
  current_consume_ = 0;
}

template <typename T>
void Task_queue<T>::push(T const& task) {
  uint32_t const current = top_++;

  tasks_[current] = task;
}

template <typename T>
bool Task_queue<T>::pop(T& task) {
  // uint32_t const current = current_consume_++;
  uint32_t const current = current_consume_.fetch_add(1, std::memory_order_relaxed);

  if (current < top_) {
    task = tasks_[current];
    return true;
  }

  return false;
}

}  // namespace thread

#endif
