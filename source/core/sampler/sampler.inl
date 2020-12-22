#ifndef SU_CORE_SAMPLER_SAMPLER_INL
#define SU_CORE_SAMPLER_SAMPLER_INL

#include "base/memory/align.hpp"
#include "sampler.hpp"

namespace sampler {

template <typename T>
Typed_pool<T>::Typed_pool(uint32_t num_samplers)
    : Pool(num_samplers), samplers_(memory::allocate_aligned<T>(num_samplers)) {}

template <typename T>
Typed_pool<T>::~Typed_pool() {
    for (uint32_t i = 0, len = num_samplers_; i < len; ++i) {
        samplers_[i].~T();
    }

    std::free(samplers_);
}

template <typename T>
Sampler* Typed_pool<T>::get(uint32_t id, uint32_t num_dimensions_2D,
                            uint32_t num_dimensions_1D) const {
    return new (&samplers_[id]) T(num_dimensions_2D, num_dimensions_1D);
}

}  // namespace sampler

#endif
