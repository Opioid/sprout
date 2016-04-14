#pragma once

#include <condition_variable>
#include <functional>
#include <thread>
#include <vector>

namespace thread {

class Pool {
public:

	using Parallel_program = std::function<void(uint32_t)>;
	using Range_program	   = std::function<void(int32_t, int32_t)>;

	Pool(uint32_t num_threads);
	~Pool();

	uint32_t num_threads() const;

	void run(Parallel_program program);
	void run_range(Range_program program, int32_t begin, int32_t end);

private:

	void wake_all();
	void wake_all(int32_t begin, int32_t end);

	void wait_all();

	struct Unique {
		int32_t begin;
		int32_t end;
		std::condition_variable wake_signal;
		std::condition_variable done_signal;
		std::mutex mutex;
		bool wake;
	};

	struct Shared {
		Parallel_program parallel_program;
		Range_program    range_program;
		bool end;
	};

	uint32_t num_threads_;

	Shared shared_;

	std::vector<Unique> uniques_;
	std::vector<std::thread> threads_;

	static void loop(uint32_t id, Unique& unique, const Shared& shared);
};

}
