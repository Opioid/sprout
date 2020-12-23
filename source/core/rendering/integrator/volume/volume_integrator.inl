#ifndef SU_CORE_RENDERING_INTEGRATOR_VOLUME_INTEGRATOR_INL
#define SU_CORE_RENDERING_INTEGRATOR_VOLUME_INTEGRATOR_INL

#include "base/memory/align.hpp"
#include "volume_integrator.hpp"

namespace rendering::integrator::volume {

template <typename T>
Typed_pool<T>::Typed_pool(uint32_t num_integrators)
    : Pool(num_integrators), integrators_(memory::allocate_aligned<T>(num_integrators)) {}

template <typename T>
Typed_pool<T>::~Typed_pool() {
    for (uint32_t i = 0, len = num_integrators_; i < len; ++i) {
        integrators_[i].~T();
    }

    std::free(integrators_);
}

}  // namespace rendering::integrator::volume

#endif
