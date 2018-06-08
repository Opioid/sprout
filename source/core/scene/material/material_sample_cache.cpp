#include "material_sample_cache.hpp"
#include "base/memory/align.hpp"

namespace scene::material {

Sample_cache::~Sample_cache() {
    memory::free_aligned(buffer_);
}

void Sample_cache::init(uint32_t max_sample_size) {
    buffer_size_ = max_sample_size;
    buffer_      = memory::allocate_aligned<char>(max_sample_size);
}

size_t Sample_cache::num_bytes() const {
    return sizeof(*this) + buffer_size_ * sizeof(char);
}

}  // namespace scene::material
