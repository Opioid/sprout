#ifndef SU_BASE_RANDOM_SHUFFLE_HPP
#define SU_BASE_RANDOM_SHUFFLE_HPP

#include "generator.inl"
#include <utility>

namespace rnd {

template<typename T>
void shuffle(T* data, uint32_t count, Generator& rng) {
	// This is a copy of
	// http://en.cppreference.com/w/cpp/algorithm/random_shuffle
	for (uint32_t i = count - 1; i > 0; --i) {
		uint32_t const other = rng.random_uint() % (i + 1);
		std::swap(data[i], data[other]);
	}
}

}

#endif
