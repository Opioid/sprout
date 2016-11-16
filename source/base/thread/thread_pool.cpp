#include "thread_pool.hpp"
#include <cmath>

namespace thread {

Pool::Pool(uint32_t num_threads) :
	num_threads_(num_threads),
	uniques_(num_threads),
	threads_(num_threads) {
	shared_.end  = false;

	for (uint32_t i = 0; i < num_threads; ++i) {
		uniques_[i].wake = false;
	}

	for (uint32_t i = 0; i < num_threads; ++i) {
		threads_[i] = std::thread(loop, i, std::ref(uniques_[i]), std::cref(shared_));
	}
}

Pool::~Pool() {
	shared_.end = true;

	wake_all();

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

	wake_all();

	wait_all();
}

void Pool::run_range(Range_program program, int32_t begin, int32_t end) {
	shared_.range_program = program;
	shared_.parallel_program = nullptr;

	wake_all(begin, end);

	wait_all();
}

void Pool::wake_all() {
	for (auto& u : uniques_) {
		std::unique_lock<std::mutex> lock(u.mutex);
		u.wake = true;
		lock.unlock();
		u.wake_signal.notify_one();
	}
}

void Pool::wake_all(int32_t begin, int32_t end) {
	float range = static_cast<float>(end - begin);

	int32_t step = static_cast<int32_t>(std::ceil(range / static_cast<float>(threads_.size())));

	int32_t b = 0;
	int32_t e = begin;

	for (auto& u : uniques_) {
		b = e;
		e += step;

		std::unique_lock<std::mutex> lock(u.mutex);
		u.begin = b;
		u.end   = std::min(e, end);
		u.wake  = true;
		lock.unlock();
		u.wake_signal.notify_one();
	}
}

void Pool::wait_all() {
	for (auto& u : uniques_) {
		std::unique_lock<std::mutex> lock(u.mutex);
		u.done_signal.wait(lock, [&u]{ return !u.wake; });
	}
}

void Pool::loop(uint32_t id, Unique& unique, const Shared& shared) {
	for (;;) {
		std::unique_lock<std::mutex> lock(unique.mutex);
		unique.wake_signal.wait(lock, [&unique]{ return unique.wake; });

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
		unique.done_signal.notify_one();
	}
}

}
