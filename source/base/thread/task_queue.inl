#pragma once

#include "task_queue.hpp"
#include "memory/align.hpp"

namespace thread {

template<typename T>
Task_queue<T>::Task_queue(uint32_t num_tasks) :
	num_tasks_(num_tasks),
	tasks_(memory::allocate_aligned<T>(num_tasks)),
	current_consume_(num_tasks) {}

template<typename T>
Task_queue<T>::~Task_queue() {
	memory::free_aligned(tasks_);
}

template<typename T>
uint32_t Task_queue<T>::size() const {
	return num_tasks_;
}

template<typename T>
void Task_queue<T>::restart() {
	current_consume_ = 0;
}

template<typename T>
bool Task_queue<T>::pop(T& task) {
	const uint32_t current = current_consume_++;

	if (current < num_tasks_) {
		task = tasks_[current];
		return true;
	}

	return false;
}

template<typename T>
void Task_queue<T>::push(const T& task) {
	const uint32_t current = num_tasks_ - current_consume_--;

	tasks_[current] = task;
}

}
