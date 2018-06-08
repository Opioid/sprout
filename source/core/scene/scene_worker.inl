#pragma once

#include "material/material_sample_cache.inl"
#include "scene_worker.hpp"

namespace scene {

template <typename T>
T& Worker::sample() const {
    return sample_cache_.get<T>();
}

}  // namespace scene
