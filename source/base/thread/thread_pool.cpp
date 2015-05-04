#include "thread_pool.hpp"

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
	run();

	for (auto& t : threads_) {
		t.join();
	}
}

void Pool::run() {
	for (size_t i = 0; i < uniques_.size(); ++i) {
		uniques_[i].wake = true;
	}

	shared_.wake_signal.notify_all();


	for (size_t i = 0; i < uniques_.size(); ++i) {
		std::unique_lock<std::mutex> locker(shared_.lock);

		uniques_[i].done_signal.wait(locker);
	}

	std::cout << "run done" << std::endl;
}

void Pool::loop(Unique& unique, Shared& shared) {

	for (;;) {
		std::unique_lock<std::mutex> locker(shared.lock);

		while (!unique.wake) // used to avoid spurious wakeups
		{
			shared.wake_signal.wait(locker);
		}

		if (shared.end) {
			break;
		}

		std::cout << "a" << std::endl;

		unique.wake = false;
		unique.done_signal.notify_one();
	}
}

}
