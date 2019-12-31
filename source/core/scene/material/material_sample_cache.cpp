#include "material_sample_cache.hpp"
#include "base/memory/align.hpp"

namespace scene::material {

Sample_cache::Sample_cache(uint32_t max_sample_size) noexcept
    : max_sample_size_(max_sample_size),
      buffer_(memory::allocate_aligned<char>(max_sample_size_)) {}

Sample_cache::~Sample_cache() noexcept {
    memory::free_aligned(buffer_);
}

}  // namespace scene::material
