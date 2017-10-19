#pragma once

#include <atomic>

namespace thread {

template<typename T>
class Task_queue {

public:

	Task_queue(uint32_t num_tasks);
	~Task_queue();

	uint32_t size() const;

	void restart();

	bool pop(T& task);

protected:

	void push(const T& task);

private:

	uint32_t num_tasks_;
	T* tasks_;

	std::atomic<uint32_t> current_consume_;
};

}
