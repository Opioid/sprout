#include "canopy.hpp"
#include "shape_sample.hpp"
#include "geometry/shape_intersection.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "sampler/sampler.hpp"
#include "base/math/sampling.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/ray.inl"
#include "base/math/bounding/aabb.inl"

namespace scene { namespace shape {

Canopy::Canopy() {
	aabb_.set_min_max(math::float3::identity, math::float3::identity);
}

bool Canopy::intersect(const Composed_transformation& transformation, math::Oray& ray,
					   const math::float2& /*bounds*/, Node_stack& /*node_stack*/,
					   Intersection& intersection) const {
	if (ray.max_t >= 1000.f) {
		intersection.epsilon = 5e-4f;

		intersection.p = ray.point(1000.f);
		intersection.t = transformation.rotation.x;
		intersection.b = transformation.rotation.y;
		intersection.n = -ray.direction;
		intersection.geo_n = intersection.n;
		intersection.part = 0;

		ray.max_t = 1000.f;
		return true;
	}

	return false;
}

bool Canopy::intersect_p(const Composed_transformation& /*transformation*/, const math::Oray& /*ray*/,
						 const math::float2& /*bounds*/, Node_stack& /*node_stack*/) const {
	// Implementation for this is not really needed, so just skip it
	return false;
}

float Canopy::opacity(const Composed_transformation& /*transformation*/, const math::Oray& /*ray*/,
					  const math::float2& /*bounds*/, Node_stack& /*node_stack*/,
					  const material::Materials& /*materials*/, const image::sampler::Sampler_2D& /*sampler*/) const {
	// Implementation for this is not really needed, so just skip it
	return 1.f;
}

void Canopy::sample(uint32_t /*part*/, const Composed_transformation& /*transformation*/, float /*area*/,
					const math::float3& /*p*/, const math::float3& n,
					sampler::Sampler& sampler, Sample& sample) const {
	math::float3 x, y;
	math::coordinate_system(n, x, y);

	math::float2 uv = sampler.generate_sample_2d();
	math::float3 dir = math::sample_oriented_hemisphere_uniform(uv, x, y, n);

	sample.wi  = dir;
	sample.t   = 1000.f;
	sample.pdf = 1.f / (2.f * math::Pi);
}

float Canopy::pdf(uint32_t /*part*/, const Composed_transformation& /*transformation*/, float /*area*/,
				  const math::float3& /*p*/, const math::float3& /*wi*/) const {
	return 1.f / (2.f * math::Pi);
}

float Canopy::area(uint32_t /*part*/, const math::float3& /*scale*/) const {
	return 4.f * math::Pi;
}

bool Canopy::is_finite() const {
	return false;
}

}}

