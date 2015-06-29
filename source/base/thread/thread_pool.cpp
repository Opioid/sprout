#include "thread_pool.hpp"
#include <algorithm>

namespace thread {

Pool::Pool(uint32_t num_threads) : num_threads_(num_threads), uniques_(num_threads), threads_(num_threads) {
	shared_.end  = false;

	for (uint32_t i = 0; i < num_threads; ++i) {
		uniques_[i].wake = false;
	}

	for (uint32_t i = 0; i < num_threads; ++i) {
		threads_[i] = std::thread(loop, i, std::ref(uniques_[i]), std::ref(shared_));
	}
}

Pool::~Pool() {
	shared_.end = true;

	wake_all(0, 0);

	for (auto& t : threads_) {
		t.join();
	}
}

uint32_t Pool::num_threads() const {
	return num_threads_;
}

void Pool::run(Parallel_program program) {
	shared_.parallel_program = program;
	shared_.range_program = nullptr;

	wake_all(0, 0);

	wait_all();
}

void Pool::run_range(Range_program program, uint32_t begin, uint32_t end) {
	shared_.range_program = program;
	shared_.parallel_program = nullptr;

	wake_all(begin, end);

	wait_all();
}

void Pool::wake_all(uint32_t begin, uint32_t end) {
	float range = static_cast<float>(end - begin);

	uint32_t step = static_cast<uint32_t>(std::ceil(range / static_cast<float>(threads_.size())));

	{
	//	std::lock_guard<std::mutex> lock(shared_.mutex);

		uint32_t b = 0;
		uint32_t e = begin;
		for (size_t i = 0, len = uniques_.size(); i < len; ++i) {
			b = e;
			e += step;

			uniques_[i].begin = b;
			uniques_[i].end   = std::min(e, end);

			uniques_[i].wake = true;

			uniques_[i].wake_signal.notify_one();
		}
	}

//	shared_.wake_signal.notify_all();
}

void Pool::wait_all() {
	for (size_t i = 0, len = uniques_.size(); i < len; ++i) {
		std::unique_lock<std::mutex> lock(uniques_[i].mutex);
		uniques_[i].done_signal.wait(lock, [this, i]{return !uniques_[i].wake;});
	}
}

void Pool::loop(uint32_t id, Unique& unique, Shared& shared) {
	for (;;) {
//		std::unique_lock<std::mutex> lock(shared.mutex);
//		shared.wake_signal.wait(lock, [&unique]{return unique.wake;});
//		lock.unlock();

		std::unique_lock<std::mutex> lock(unique.mutex);
		unique.wake_signal.wait(lock, [&unique]{return unique.wake;});
	//	lock.unlock();

		if (shared.end) {
			break;
		}

		if (shared.range_program) {
			shared.range_program(unique.begin, unique.end);
		} else if (shared.parallel_program) {
			shared.parallel_program(id);
		}

		unique.wake = false;
		lock.unlock();
		unique.done_signal.notify_all();
	}
}

}
