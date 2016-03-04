#include "canopy.hpp"
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

Canopy::Canopy() {
	aabb_.set_min_max(math::float3_identity, math::float3_identity);
}

bool Canopy::intersect(const entity::Composed_transformation& transformation, math::Oray& ray,
					   Node_stack& /*node_stack*/, Intersection& intersection) const {
	if (ray.max_t >= 10000.f) {
		intersection.epsilon = 5e-4f;

		intersection.p = ray.point(10000.f);
		intersection.t = transformation.rotation.x3;
		intersection.b = transformation.rotation.y3;
		intersection.n = -ray.direction;
		intersection.geo_n = intersection.n;
		intersection.part = 0;

		math::float3 xyz = math::normalized(
					math::transform_vector_transposed(ray.direction, transformation.rotation));
		intersection.uv = math::float2(std::atan2(xyz.x, xyz.z) * math::Pi_inv * 0.5f + 0.5f,
									   std::acos(xyz.y) * math::Pi_inv);

		ray.max_t = 10000.f;
		return true;
	}

	return false;
}

bool Canopy::intersect_p(const entity::Composed_transformation& /*transformation*/, const math::Oray& /*ray*/,
						 Node_stack& /*node_stack*/) const {
	// Implementation for this is not really needed, so just skip it
	return false;
}

float Canopy::opacity(const entity::Composed_transformation& /*transformation*/, const math::Oray& /*ray*/,
					  float /*time*/, const material::Materials& /*materials*/,
					  Worker& /*worker*/, material::Sampler_settings::Filter /*filter*/) const {
	// Implementation for this is not really needed, so just skip it
	return 0.f;
}

void Canopy::sample(uint32_t /*part*/, const entity::Composed_transformation& transformation, float /*area*/,
					const math::float3& /*p*/, const math::float3& n, bool /*two_sided*/,
					sampler::Sampler& sampler, Node_stack& /*node_stack*/, Sample& sample) const {
	math::float3 x, y;
	math::coordinate_system(n, x, y);

	math::float2 uv = sampler.generate_sample_2D();
	math::float3 dir = math::sample_oriented_hemisphere_uniform(uv, x, y, n);

	sample.wi = dir;

	math::float3 xyz = math::transform_vector_transposed(dir, transformation.rotation);
	sample.uv = math::float2(std::atan2(xyz.x, xyz.z) * math::Pi_inv * 0.5f + 0.5f,
							 std::acos(xyz.y) * math::Pi_inv);

	sample.t   = 10000.f;
	sample.pdf = 1.f / (2.f * math::Pi);
}

void Canopy::sample(uint32_t /*part*/, const entity::Composed_transformation& transformation, float /*area*/,
					const math::float3& /*p*/, bool /*two_sided*/,
					sampler::Sampler& sampler, Node_stack& /*node_stack*/, Sample& sample) const {
	math::float2 uv = sampler.generate_sample_2D();
	math::float3 dir = math::sample_sphere_uniform(uv);

	sample.wi = dir;

	math::float3 xyz = math::transform_vector_transposed(dir, transformation.rotation);
	sample.uv = math::float2(std::atan2(xyz.x, xyz.z) * math::Pi_inv * 0.5f + 0.5f,
							 std::acos(xyz.y) * math::Pi_inv);

	sample.t   = 10000.f;
	sample.pdf = 1.f / (4.f * math::Pi);
}

void Canopy::sample(uint32_t /*part*/, const entity::Composed_transformation& transformation, float /*area*/,
					const math::float3& /*p*/, math::float2 uv, Sample& sample) const {
	float phi   = (-uv.x + 0.75f) * 2.f * math::Pi;
	float theta = uv.y * math::Pi;

	float sin_theta = std::sin(theta);
	float cos_theta = std::cos(theta);
	float sin_phi   = std::sin(phi);
	float cos_phi   = std::cos(phi);

	math::float3 dir(sin_theta * cos_phi, cos_theta, sin_theta * sin_phi);

	sample.wi = math::transform_vector(dir, transformation.rotation);
	sample.uv = uv;
	sample.t  = 10000.f;
	sample.pdf = 1.f / (4.f * math::Pi);
}

void Canopy::sample(uint32_t /*part*/, const entity::Composed_transformation& transformation, float /*area*/,
					const math::float3& /*p*/, const math::float3& wi, Sample& sample) const {
	math::float3 xyz = math::normalized(math::transform_vector_transposed(wi, transformation.rotation));
	sample.uv = math::float2(std::atan2(xyz.x, xyz.z) * math::Pi_inv * 0.5f + 0.5f, std::acos(xyz.y) * math::Pi_inv);

	sample.pdf = 1.f / (4.f * math::Pi);
}

float Canopy::pdf(uint32_t /*part*/, const entity::Composed_transformation& /*transformation*/, float /*area*/,
				  const math::float3& /*p*/, const math::float3& /*wi*/, bool /*two_sided*/, bool total_sphere,
				  Node_stack& /*node_stack*/) const {
	if (total_sphere) {
		return 1.f / (4.f * math::Pi);
	} else {
		return 1.f / (2.f * math::Pi);
	}
}

float Canopy::area(uint32_t /*part*/, const math::float3& /*scale*/) const {
	return 4.f * math::Pi;
}

bool Canopy::is_finite() const {
	return false;
}

}}
