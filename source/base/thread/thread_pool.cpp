#include "thread_pool.hpp"
#include <algorithm>
#include <iostream>

namespace thread {

Pool::Pool(uint32_t num_threads) : uniques_(num_threads), threads_(num_threads) {
	shared_.end  = false;

	for (uint32_t i = 0; i < num_threads; ++i) {
		uniques_[i].wake = false;
	}

	for (uint32_t i = 0; i < num_threads; ++i) {
		threads_[i] = std::thread(loop, std::ref(uniques_[i]), std::ref(shared_));
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

void Pool::run_range(Range_program program, uint32_t begin, uint32_t end) {
	shared_.range_program = program;

	wake_all(begin, end);

	wait_all();
}

void Pool::wake_all(uint32_t begin, uint32_t end) {
	float range = static_cast<float>(end - begin);

	uint32_t step = static_cast<uint32_t>(std::ceil(range / static_cast<float>(threads_.size())));

	uint32_t b = 0;
	uint32_t e = begin;
	for (size_t i = 0; i < uniques_.size(); ++i) {
		b = e;
		e += step;

		uniques_[i].begin = b;
		uniques_[i].end   = std::min(e, end);

		uniques_[i].wake = true;
	}

	shared_.wake_signal.notify_all();
}

void Pool::wait_all() {
	for (size_t i = 0; i < uniques_.size(); ++i) {
		std::unique_lock<std::mutex> locker(uniques_[i].lock);
		uniques_[i].done_signal.wait(locker, [&]{return !uniques_[i].wake;});
	}
}

void Pool::loop(Unique& unique, Shared& shared) {
	for (;;) {
		std::unique_lock<std::mutex> locker(unique.lock);

		shared.wake_signal.wait(locker, [&]{return unique.wake;});

		if (shared.end) {
			break;
		}

		shared.range_program(unique.begin, unique.end);

		unique.wake = false;
		locker.unlock();
		unique.done_signal.notify_all();
	}
}

}
