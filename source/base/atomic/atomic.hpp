#pragma once

#include <atomic>

namespace atomic {

inline void add(float& a, float b) {
/*	std::atomic<float> aa(a);
	auto current = aa.load();

	while (!aa.compare_exchange_weak(current, current + b));

	a = aa.load();
	*/

	std::atomic<float> aa(a);
	auto current = aa.load(std::memory_order_relaxed);
	do {
		a = current;
	} while (!aa.compare_exchange_weak(current, current + b,
									   std::memory_order_release,
									   std::memory_order_relaxed));

	a = aa.load();
}

}
