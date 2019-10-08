#ifndef SU_CORE_SCENE_WORKER_INL
#define SU_CORE_SCENE_WORKER_INL

#include "material/material_sample_cache.inl"
#include "scene_worker.hpp"

namespace scene {

inline Scene const& Worker::scene() const noexcept {
    return *scene_;
}

template <typename T>
T& Worker::sample() const noexcept {
    return sample_cache_.get<T>();
}

}  // namespace scene

#endif
