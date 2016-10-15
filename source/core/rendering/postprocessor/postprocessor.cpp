#include "postprocessor.hpp"
#include "base/math/vector.inl"
#include "base/thread/thread_pool.hpp"

namespace rendering { namespace postprocessor {

Postprocessor::Postprocessor(uint32_t num_passes) : num_passes_(num_passes) {}

Postprocessor::~Postprocessor() {}

void Postprocessor::apply(const image::Image_float_4& source,
						  image::Image_float_4& destination,
						  thread::Pool& pool) {
	for (uint32_t p = 0; p < num_passes_; ++p) {
		pool.run_range([this, p, &source, &destination](int32_t begin, int32_t end) {
			apply(begin, end, p, source, destination); }, 0, destination.area());
	}
}

}}

