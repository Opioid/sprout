#include "material_sample_cache.hpp"
#include "base/memory/align.hpp"

namespace scene::material {

Sample_cache::~Sample_cache() noexcept {
    memory::free_aligned(buffer_);
}

void Sample_cache::init(uint32_t max_sample_size, uint32_t max_depth) noexcept {
    max_sample_size_ = max_sample_size;
    buffer_size_     = max_sample_size * max_depth;
    buffer_          = memory::allocate_aligned<char>(buffer_size_);
}

size_t Sample_cache::num_bytes() const noexcept {
    return sizeof(*this) + buffer_size_ * sizeof(char);
}

}  // namespace scene::material
