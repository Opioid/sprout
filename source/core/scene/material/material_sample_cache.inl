#ifndef SU_CORE_SCENE_MATERIAL_SAMPLE_CACHE_INL
#define SU_CORE_SCENE_MATERIAL_SAMPLE_CACHE_INL

#include "material_sample_cache.hpp"

namespace scene::material {

template <typename T>
T& Sample_cache::get(uint32_t sample_level) noexcept {
    return *new (buffer_ + max_sample_size_ * sample_level) T;
}

}  // namespace scene::material

#endif
