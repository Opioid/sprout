#ifndef SU_BASE_RANDOM_SHUFFLE_HPP
#define SU_BASE_RANDOM_SHUFFLE_HPP

#include "generator.inl"

#include <utility>

namespace rnd {

// This is a copy of
// http://en.cppreference.com/w/cpp/algorithm/random_shuffle
template <typename T>
void biased_shuffle(T* data, uint32_t count, Generator& rng) noexcept {
    //	for (uint32_t i = count - 1; i > 0; --i) {
    //		uint32_t const other = rng.random_uint() % (i + 1);

    //		std::swap(data[i], data[other]);
    //	}

    // Divisionless optimization with slight bias from
    // https://lemire.me/blog/2016/06/30/fast-random-shuffling/
    // (Upper variant has bias as well!)
    // More related information:
    // http://www.pcg-random.org/posts/bounded-rands.html
    for (uint64_t i = uint64_t(count - 1); i > 0; --i) {
        uint64_t const r     = uint64_t(rng.random_uint());
        uint64_t const m     = r * (i + 1);
        uint64_t const other = m >> 32;

        std::swap(data[i], data[other]);
    }
}

}  // namespace rnd

#endif
