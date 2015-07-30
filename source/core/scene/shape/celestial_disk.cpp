#include "celestial_disk.hpp"
#include "shape_sample.hpp"
#include "geometry/shape_intersection.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "sampler/sampler.hpp"
#include "base/math/sampling/sampling.inl"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/ray.inl"
#include "base/math/bounding/aabb.inl"

namespace scene { namespace shape {

Celestial_disk::Celestial_disk() {
	aabb_.set_min_max(math::float3::identity, math::float3::identity);
}

bool Celestial_disk::intersect(const entity::Composed_transformation& transformation, math::Oray& ray,
							   const math::float2& /*bounds*/, Node_stack& /*node_stack*/,
							   Intersection& intersection) const {
	const math::float3& v = transformation.rotation.z;
	float b = -dot(v, ray.direction);
	float radius = transformation.scale.x;
	float det = (b * b) - dot(v, v) + (radius * radius);

	if (det > 0.f && ray.max_t >= 1000.f) {
		intersection.epsilon = 5e-4f;

		intersection.p = ray.point(999.9f);
		intersection.t = transformation.rotation.x;
		intersection.b = transformation.rotation.y;
		intersection.n = transformation.rotation.z;
		intersection.geo_n = transformation.rotation.z;
		intersection.part = 0;

		ray.max_t = 999.9f;
		return true;
	}

	return false;
}

bool Celestial_disk::intersect_p(const entity::Composed_transformation& /*transformation*/, const math::Oray& /*ray*/,
								 const math::float2& /*bounds*/, Node_stack& /*node_stack*/) const {
	// Implementation for this is not really needed, so just skip it
	return false;
}

float Celestial_disk::opacity(const entity::Composed_transformation& /*transformation*/, const math::Oray& /*ray*/,
							  const math::float2& /*bounds*/, Node_stack& /*node_stack*/,
							  const material::Materials& /*materials*/, const image::texture::sampler::Sampler_2D& /*sampler*/) const {
	// Implementation for this is not really needed, so just skip it
	return 0.f;
}

void Celestial_disk::sample(uint32_t /*part*/, const entity::Composed_transformation& transformation, float area,
							const math::float3& /*p*/, const math::float3& /*n*/,
							sampler::Sampler& sampler, Sample& sample) const {
	math::float2 r2 = sampler.generate_sample_2D();
	math::float2 xy = math::sample_disk_concentric(r2);

	math::float3 ls = math::float3(xy, 0.f);
	math::float3 ws = transformation.scale.x * math::transform_vector(transformation.rotation, ls);

	sample.wi = math::normalized(ws - transformation.rotation.z);
	sample.t = 1000.f;
	sample.pdf = 1.f / area;
}

void Celestial_disk::sample(uint32_t /*part*/, const entity::Composed_transformation& /*transformation*/, float /*area*/,
							const math::float3& /*p*/, math::float2 /*uv*/, Sample& /*sample*/) const {}

void Celestial_disk::sample(uint32_t /*part*/, const entity::Composed_transformation& /*transformation*/, float /*area*/,
							const math::float3& /*p*/, const math::float3& /*wi*/, Sample& /*sample*/) const {}

float Celestial_disk::pdf(uint32_t /*part*/, const entity::Composed_transformation& /*transformation*/, float area,
						  const math::float3& /*p*/, const math::float3& /*wi*/) const {
	return 1.f / area;
}

float Celestial_disk::area(uint32_t /*part*/, const math::float3& scale) const {
	return math::Pi * scale.x * scale.x;
}

bool Celestial_disk::is_finite() const {
	return false;
}

}}

