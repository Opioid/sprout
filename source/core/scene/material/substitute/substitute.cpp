#include "substitute.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace substitute {

math::float3 Sample::evaluate(const math::float3& wi) const {
	float n_dot_wi = std::max(math::dot(n_, wi), 0.00001f);


	return n_dot_wi * color_;
}

void Sample::set(const math::float3& color) {
	color_ = color;
}

Substitute::Substitute(Sample_cache<Sample>& cache) : Material(cache) {}

}}}
