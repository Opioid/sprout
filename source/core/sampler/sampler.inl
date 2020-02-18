#ifndef SU_CORE_SAMPLER_SAMPLER_INL
#define SU_CORE_SAMPLER_SAMPLER_INL

#include "base/memory/align.hpp"
#include "sampler.hpp"

#include <cstring>

namespace sampler {

template <typename T>
Typed_pool<T>::Typed_pool(uint32_t num_samplers)
    : Pool(num_samplers), samplers_(memory::allocate_aligned<T>(num_samplers)) {
    std::memset(static_cast<void*>(samplers_), 0, sizeof(T) * num_samplers);
}

template <typename T>
Typed_pool<T>::~Typed_pool() {
    for (uint32_t i = 0, len = num_samplers_; i < len; ++i) {
        memory::destroy(&samplers_[i]);
    }

    memory::free_aligned(samplers_);
}

template <typename T>
Sampler* Typed_pool<T>::get(uint32_t id, rnd::Generator& rng) const {
    if (uint32_t const zero = 0; 0 == std::memcmp(&zero, static_cast<void*>(&samplers_[id]), 4)) {
        return new (&samplers_[id]) T(rng);
    }

    return &samplers_[id];
}

}  // namespace sampler

#endif
