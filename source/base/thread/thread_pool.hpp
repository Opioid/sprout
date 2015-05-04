#pragma once

#include <vector>
#include <condition_variable>
#include <thread>

namespace thread {

class Pool {
public:

	Pool(uint32_t num_threads);
	~Pool();

	void run();

private:

	struct Unique {
		std::condition_variable done_signal;
		bool wake;
	};

	struct Shared {
		std::condition_variable wake_signal;
		std::mutex lock;
		bool end;
	};

	Shared shared_;

	static void loop(Unique& unique, Shared& shared);

	std::vector<Unique> uniques_;
	std::vector<std::thread> threads_;


};

}
