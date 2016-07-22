#include "infinite_sphere.hpp"
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

Infinite_sphere::Infinite_sphere() {
	aabb_.set_min_max(math::float3_identity, math::float3_identity);
}

bool Infinite_sphere::intersect(const Transformation& transformation,
								math::Oray& ray, Node_stack& /*node_stack*/,
								Intersection& intersection) const {
	if (ray.max_t >= 1000000.f) {
		intersection.epsilon = 5e-4f;

		intersection.p = ray.point(1000000.f);
		intersection.t = transformation.rotation.v3.x;
		intersection.b = transformation.rotation.v3.y;

		float3 n = -ray.direction;
		intersection.n = n;
		intersection.geo_n = n;
		intersection.part = 0;

		float3 xyz = math::transform_vector_transposed(ray.direction,
															 transformation.rotation);
		xyz = math::normalized(xyz);
		intersection.uv.x = std::atan2(xyz.x, xyz.z) * math::Pi_inv * 0.5f + 0.5f;
		intersection.uv.y = std::acos(xyz.y) * math::Pi_inv;

		ray.max_t = 1000000.f;
		return true;
	}

	return false;
}

bool Infinite_sphere::intersect_p(const Transformation& /*transformation*/,
								  const math::Oray& /*ray*/, Node_stack& /*node_stack*/) const {
	// Implementation for this is not really needed, so just skip it
	return false;
}

float Infinite_sphere::opacity(const Transformation& /*transformation*/,
							   const math::Oray& /*ray*/, float /*time*/,
							   const material::Materials& /*materials*/, Worker& /*worker*/,
							   Sampler_filter /*filter*/) const {
	// Implementation for this is not really needed, so just skip it
	return 0.f;
}

void Infinite_sphere::sample(uint32_t /*part*/, const Transformation& transformation,
							 float /*area*/, const float3& /*p*/, const float3& n,
							 bool /*two_sided*/, sampler::Sampler& sampler,
							 Node_stack& /*node_stack*/, Sample& sample) const {
	float3 x, y;
	math::coordinate_system(n, x, y);

	float2 uv = sampler.generate_sample_2D();
	float3 dir = math::sample_oriented_hemisphere_uniform(uv, x, y, n);

	sample.wi = dir;

	float3 xyz = math::transform_vector_transposed(dir, transformation.rotation);
	xyz = math::normalized(xyz);
	sample.uv.x = std::atan2(xyz.x, xyz.z) * math::Pi_inv * 0.5f + 0.5f;
	sample.uv.y = std::acos(xyz.y) * math::Pi_inv;

	sample.t   = 1000000.f;
	sample.pdf = 1.f / (2.f * math::Pi);
}

void Infinite_sphere::sample(uint32_t /*part*/, const Transformation& transformation,
							 float /*area*/, const float3& /*p*/, bool /*two_sided*/,
							 sampler::Sampler& sampler, Node_stack& /*node_stack*/,
							 Sample& sample) const {
	float2 uv = sampler.generate_sample_2D();
	float3 dir = math::sample_sphere_uniform(uv);

	sample.wi = dir;

	float3 xyz = math::transform_vector_transposed(dir, transformation.rotation);
	xyz = math::normalized(xyz);
	sample.uv.x = std::atan2(xyz.x, xyz.z) * math::Pi_inv * 0.5f + 0.5f;
	sample.uv.y = std::acos(xyz.y) * math::Pi_inv;

	sample.t   = 1000000.f;
	sample.pdf = 1.f / (4.f * math::Pi);
}

void Infinite_sphere::sample(uint32_t /*part*/, const Transformation& transformation,
							 float /*area*/, const float3& /*p*/,
							 float2 uv, Sample& sample) const {
	float phi   = (-uv.x + 0.75f) * 2.f * math::Pi;
	float theta = uv.y * math::Pi;

	float sin_phi   = std::sin(phi);
	float cos_phi   = std::cos(phi);
	float sin_theta = std::sin(theta);
	float cos_theta = std::cos(theta);

	float3 dir(sin_phi * sin_theta, cos_theta, cos_phi * sin_theta);

	sample.wi = math::transform_vector(dir, transformation.rotation);
	sample.uv = uv;
	sample.t  = 1000000.f;
	sample.pdf = 1.f / (4.f * math::Pi);
}

void Infinite_sphere::sample(uint32_t /*part*/, const Transformation& transformation,
							 float /*area*/, const float3& /*p*/,
							 const float3& wi, Sample& sample) const {
	float3 xyz = math::transform_vector_transposed(wi, transformation.rotation);
	xyz = math::normalized(xyz);
	sample.uv.x = std::atan2(xyz.x, xyz.z) * math::Pi_inv * 0.5f + 0.5f;
	sample.uv.y = std::acos(xyz.y) * math::Pi_inv;

	sample.pdf = 1.f / (4.f * math::Pi);
}

float Infinite_sphere::pdf(uint32_t /*part*/, const Transformation& /*transformation*/,
						   float /*area*/, const float3& /*p*/,
						   const float3& /*wi*/, bool /*two_sided*/,
						   bool total_sphere, Node_stack& /*node_stack*/) const {
	if (total_sphere) {
		return 1.f / (4.f * math::Pi);
	} else {
		return 1.f / (2.f * math::Pi);
	}
}

float Infinite_sphere::area(uint32_t /*part*/, const float3& /*scale*/) const {
	return 4.f * math::Pi;
}

bool Infinite_sphere::is_finite() const {
	return false;
}

}}
