#include "postprocessor.hpp"
#include "base/thread/thread_pool.hpp"

namespace rendering { namespace postprocessor {

Postprocessor::~Postprocessor() {}

void Postprocessor::apply(const image::Image_float_4& source,
						  image::Image_float_4& destination,
						  thread::Pool& pool) const {
	pool.run_range([this, &source, &destination](int32_t begin, int32_t end) {
		apply(begin, end, source, destination); }, 0, destination.area());
}

}}

