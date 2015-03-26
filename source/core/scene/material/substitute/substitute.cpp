#include "substitute.hpp"

namespace scene { namespace material { namespace substitute {

math::float3 Sample::evaluate(const math::float3& wi) const {
	return color_;
}

void Sample::set(const math::float3& color) {
	color_ = color;
}

Substitute::Substitute(Sample_cache<Sample>& cache) : Material(cache) {}

}}}
