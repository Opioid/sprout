#include "celestial_disk.hpp"
#include "geometry/shape_intersection.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "sampler/sampler.hpp"
#include "base/math/sampling.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/ray.inl"

namespace scene { namespace shape {

Celestial_disk::Celestial_disk() {
	aabb_.set_min_max(math::float3::identity, math::float3::identity);
}

bool Celestial_disk::intersect(const Composed_transformation& /*transformation*/, const math::Oray& /*ray*/,
							   const math::float2& /*bounds*/, Node_stack& /*node_stack*/,
							   Intersection& /*intersection*/, float& /*hit_t*/) const {
	return false;
}

bool Celestial_disk::intersect_p(const Composed_transformation& /*transformation*/, const math::Oray& /*ray*/,
								 const math::float2& /*bounds*/, Node_stack& /*node_stack*/) const {
	return false;
}

void Celestial_disk::importance_sample(const Composed_transformation& transformation, const math::float3& /*p*/,
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

bool Celestial_disk::is_delta() const {
	return true;
}

}}

