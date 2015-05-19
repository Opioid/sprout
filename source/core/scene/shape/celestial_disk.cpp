#include "celestial_disk.hpp"
#include "geometry/shape_intersection.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "sampler/sampler.hpp"
#include "base/math/sampling.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/ray.inl"
#include "base/math/bounding/aabb.inl"

namespace scene { namespace shape {

Celestial_disk::Celestial_disk() {
	aabb_.set_min_max(math::float3::identity, math::float3::identity);
}

bool Celestial_disk::intersect(const Composed_transformation& /*transformation*/, math::Oray& /*ray*/,
							   const math::float2& /*bounds*/, Node_stack& /*node_stack*/,
							   Intersection& /*intersection*/) const {
	return false;
}

bool Celestial_disk::intersect_p(const Composed_transformation& /*transformation*/, const math::Oray& /*ray*/,
								 const math::float2& /*bounds*/, Node_stack& /*node_stack*/) const {
	return false;
}

float Celestial_disk::opacity(const Composed_transformation& /*transformation*/, const math::Oray& /*ray*/,
							  const math::float2& /*bounds*/, Node_stack& /*node_stack*/,
							  const material::Materials& /*materials*/, const image::sampler::Sampler_2D& /*sampler*/) const {
	return 0.f;
}

void Celestial_disk::importance_sample(uint32_t /*part*/, const Composed_transformation& transformation, float /*area*/, const math::float3& /*p*/,
									   sampler::Sampler& sampler, uint32_t sample_index,
									   math::float3& wi, float& t, float& pdf) const {
	math::float2 sample = sampler.generate_sample2d(sample_index);
	math::float2 xy = math::sample_disk_concentric(sample);

	math::float3 ls = math::float3(xy, 0.f);
	math::float3 ws = transformation.scale.x * math::transform_vector(transformation.rotation, ls);

	wi = math::normalized(-transformation.rotation.z + ws);
	t = 1000.f;
	pdf = 1.f;
}

float Celestial_disk::area(uint32_t /*part*/, const math::float3& /*scale*/) const {
	return 1.f;
}

bool Celestial_disk::is_finite() const {
	return false;
}

}}

