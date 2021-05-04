#ifndef SU_CORE_SAMPLER_SAMPLER_INL
#define SU_CORE_SAMPLER_SAMPLER_INL

#include "base/memory/align.hpp"
#include "sampler.hpp"

#include <cstring>

namespace sampler {

template <typename T>
Typed_pool<T>::Typed_pool(uint32_t num_samplers)
    : Pool(num_samplers), samplers_(memory::allocate_aligned<T>(num_samplers)) {
    std::memset(samplers_, 0, sizeof(T) * num_samplers);
}

template <typename T>
Typed_pool<T>::~Typed_pool() {
    static uint64_t constexpr Zero = 0;

    for (uint32_t i = 0, len = num_samplers_; i < len; ++i) {
        if (std::memcmp(&samplers_[i], &Zero, sizeof(uint64_t)) != 0) {
            samplers_[i].~T();
        }
    }

    memory::free_aligned(samplers_);
}

template <typename T>
Sampler* Typed_pool<T>::create(uint32_t id, uint32_t num_dimensions_2D, uint32_t num_dimensions_1D,
                               uint32_t max_samples) const {
    return new (&samplers_[id]) T(num_dimensions_2D, num_dimensions_1D, max_samples);
}

template <typename T>
Sampler& Typed_pool<T>::get(uint32_t id) {
    return samplers_[id];
}

}  // namespace sampler

#endif
