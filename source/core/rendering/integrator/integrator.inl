#ifndef SU_RENDERING_INTEGRATOR_INTEGRATOR_INL
#define SU_RENDERING_INTEGRATOR_INTEGRATOR_INL

#include "base/memory/align.hpp"
#include "integrator.hpp"

#include <cstring>

namespace rendering::integrator {

template <typename B>
Pool<B>::~Pool() = default;

template <typename T, typename B>
Typed_pool<T, B>::Typed_pool(uint32_t num_integrators)
    : num_integrators_(num_integrators), integrators_(memory::allocate_aligned<T>(num_integrators)) {
    std::memset(integrators_, 0, sizeof(T) * num_integrators);
}

template <typename T, typename B>
Typed_pool<T, B>::~Typed_pool() {
    static uint64_t constexpr Zero = 0;

    for (uint32_t i = 0, len = num_integrators_; i < len; ++i) {
        if (std::memcmp(&integrators_[i], &Zero, sizeof(uint64_t)) != 0) {
            integrators_[i].~T();
        }
    }

    memory::free_aligned(integrators_);
}

}  // namespace rendering::integrator

#endif
