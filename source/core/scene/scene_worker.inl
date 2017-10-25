#pragma once

#include "scene_worker.hpp"
#include "material/material_sample_cache.inl"

namespace scene {

template<typename T>
T& Worker::sample() const {
	return sample_cache_.get<T>();
}

}
