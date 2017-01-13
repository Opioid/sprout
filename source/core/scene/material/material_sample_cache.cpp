#include "material_sample_cache.hpp"
#include "bssrdf.hpp"
#include "base/math/vector.inl"
#include "base/memory/align.inl"

namespace scene { namespace material {

Sample_cache::Sample_cache(uint32_t bin_size, uint32_t num_bins) :
	bin_size_(bin_size),
	buffer_(memory::allocate_aligned<char>(bin_size * num_bins)),
	bssrdfs_(new BSSRDF[num_bins]) {}

Sample_cache::~Sample_cache() {
	delete [] bssrdfs_;

	memory::free_aligned(buffer_);
}

BSSRDF& Sample_cache::bssrdf(uint32_t id) {
	return bssrdfs_[id];
}

size_t Sample_cache::num_bytes() const {
	return sizeof(*this) +
			/*num_bins_ **/ sizeof(char) +
			/*num_bins_ **/ sizeof(BSSRDF);
}

}}
