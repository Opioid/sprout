#include "thread_pool.hpp"
#include <algorithm>
#include <cmath>

namespace thread {

Pool::Pool(uint32_t num_threads) :
	num_threads_(num_threads),
	uniques_(num_threads),
	threads_(num_threads) {
	shared_.quit  = false;

	for (uint32_t i = 0; i < num_threads; ++i) {
		uniques_[i].wake = false;
	}

	for (uint32_t i = 0; i < num_threads; ++i) {
		threads_[i] = std::thread(loop, i, std::ref(uniques_[i]), std::cref(shared_));
	}

	async_.quit = false;
	async_.wake = false;
	async_thread_ = std::thread(async_loop, std::ref(async_));
}

Pool::~Pool() {
	shared_.quit = true;

	wake_all();

	for (auto& t : threads_) {
		t.join();
	}

	async_.quit = true;

	wake_async();

	async_thread_.join();
}

uint32_t Pool::num_threads() const {
	return num_threads_;
}

void Pool::run_parallel(Parallel_program program) {
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

void Pool::run_async(Async_program program) {
	wait_async();

	async_.program = program;

	wake_async();
}

void Pool::wait_async() {
	std::unique_lock<std::mutex> lock(async_.mutex);
	async_.done_signal.wait(lock, [this]{ return !async_.wake; });
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
	const float range = static_cast<float>(end - begin);
	const float num_threads = static_cast<float>(threads_.size());
	const int32_t step = static_cast<int32_t>(std::ceil(range / num_threads));

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

void Pool::wake_async() {
	std::unique_lock<std::mutex> lock(async_.mutex);
	async_.wake = true;
	lock.unlock();
	async_.wake_signal.notify_one();
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

		if (shared.quit) {
			break;
		}

		if (shared.range_program) {
			shared.range_program(id, unique.begin, unique.end);
		} else if (shared.parallel_program) {
			shared.parallel_program(id);
		}

		unique.wake = false;
		lock.unlock();
		unique.done_signal.notify_one();
	}
}

void Pool::async_loop(Async& async) {
	for (;;) {
		std::unique_lock<std::mutex> lock(async.mutex);
		async.wake_signal.wait(lock, [&async]{ return async.wake; });

		if (async.quit) {
			break;
		}

		async.program();

		async.wake = false;
		lock.unlock();
		async.done_signal.notify_one();
	}
}

}
