#include "material_sample_cache.hpp"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"

namespace scene { namespace material {

Sample_cache::Sample_cache() : buffer_size_(0), buffer_(nullptr) {}

Sample_cache::~Sample_cache() {
	memory::free_aligned(buffer_);
}

void Sample_cache::init(uint32_t max_sample_size) {
	uint32_t buffer_size = max_sample_size;
	buffer_size_ = buffer_size;
	buffer_ = memory::allocate_aligned<char>(buffer_size);
}

BSSRDF& Sample_cache::bssrdf() {
	return bssrdf_;
}

size_t Sample_cache::num_bytes() const {
	return sizeof(*this) +
			buffer_size_ * sizeof(char);
}

}}
