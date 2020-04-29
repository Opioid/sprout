#ifndef SU_CORE_SCENE_MATERIAL_SAMPLE_CACHE_INL
#define SU_CORE_SCENE_MATERIAL_SAMPLE_CACHE_INL

#include "material_sample_cache.hpp"

namespace scene::material {

inline Sample_cache::Sample_cache() = default;

inline Sample_cache::~Sample_cache() = default;

template <typename T>
T& Sample_cache::get() {
    return *new (buffer_) T;
}

}  // namespace scene::material

#endif
