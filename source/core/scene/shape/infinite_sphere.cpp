#include "infinite_sphere.hpp"
#include "shape_sample.hpp"
#include "shape_intersection.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/entity/composed_transformation.hpp"
#include "sampler/sampler.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/sampling/sampling.inl"

#include <iostream>

namespace scene { namespace shape {

Infinite_sphere::Infinite_sphere() {
	aabb_.set_min_max(math::float3_identity, math::float3_identity);
}

bool Infinite_sphere::intersect(const Transformation& transformation,
								Ray& ray, Node_stack& /*node_stack*/,
								Intersection& intersection) const {
	if (ray.max_t >= Ray_max_t) {
		intersection.epsilon = 5e-4f;

		// This is nonsense
		intersection.t = transformation.rotation.v3.x;
		intersection.b = transformation.rotation.v3.y;

		float3 xyz = math::transform_vector_transposed(ray.direction, transformation.rotation);
		xyz = math::normalized(xyz);
		intersection.uv.v[0] = std::atan2(xyz.v[0], xyz.v[2]) * (math::Pi_inv * 0.5f) + 0.5f;
		intersection.uv.v[1] = std::acos(xyz.v[1]) * math::Pi_inv;

		intersection.p = ray.point(Ray_max_t);
		const float3 n = -ray.direction;
		intersection.n = n;
		intersection.geo_n = n;
		intersection.part = 0;

		ray.max_t = Ray_max_t;
		return true;
	}

	return false;
}

bool Infinite_sphere::intersect_p(const Transformation& /*transformation*/,
								  const Ray& /*ray*/, Node_stack& /*node_stack*/) const {
	// Implementation for this is not really needed, so just skip it
	return false;
}

float Infinite_sphere::opacity(const Transformation& /*transformation*/,
							   const Ray& /*ray*/, const material::Materials& /*materials*/,
							   Worker& /*worker*/, Sampler_filter /*filter*/) const {
	// Implementation for this is not really needed, so just skip it
	return 0.f;
}

float3 Infinite_sphere::thin_absorption(const Transformation& /*transformation*/,
										const Ray& /*ray*/,
										const material::Materials& /*materials*/,
										Worker& /*worker*/, Sampler_filter /*filter*/) const {
	// Implementation for this is not really needed, so just skip it
	return float3(0.f);
}

void Infinite_sphere::sample(uint32_t /*part*/, const Transformation& transformation,
							 float3_p /*p*/, float3_p n, float /*area*/, bool /*two_sided*/,
							 sampler::Sampler& sampler, uint32_t sampler_dimension,
							 Node_stack& /*node_stack*/, Sample& sample) const {
	float3 x, y;
	math::coordinate_system(n, x, y);

	const float2 uv = sampler.generate_sample_2D(sampler_dimension);
	const float3 dir = math::sample_oriented_hemisphere_uniform(uv, x, y, n);

	sample.wi = dir;

	float3 xyz = math::transform_vector_transposed(dir, transformation.rotation);
	xyz = math::normalized(xyz);
	sample.uv.v[0] = std::atan2(xyz.v[0], xyz.v[2]) * (math::Pi_inv * 0.5f) + 0.5f;
	sample.uv.v[1] = std::acos(xyz.v[1]) * math::Pi_inv;

	sample.t   = Ray_max_t;
	sample.pdf = 1.f / (2.f * math::Pi);
}

void Infinite_sphere::sample(uint32_t /*part*/, const Transformation& transformation,
							 float3_p /*p*/, float /*area*/, bool /*two_sided*/,
							 sampler::Sampler& sampler, uint32_t sampler_dimension,
							 Node_stack& /*node_stack*/, Sample& sample) const {
	const float2 uv = sampler.generate_sample_2D(sampler_dimension);
	const float3 dir = math::sample_sphere_uniform(uv);

	sample.wi = dir;

	float3 xyz = math::transform_vector_transposed(dir, transformation.rotation);
	xyz = math::normalized(xyz);
	sample.uv.v[0] = std::atan2(xyz.v[0], xyz.v[2]) * (math::Pi_inv * 0.5f) + 0.5f;
	sample.uv.v[1] = std::acos(xyz.v[1]) * math::Pi_inv;

	sample.t   = Ray_max_t;
	sample.pdf = 1.f / (4.f * math::Pi);
}

float Infinite_sphere::pdf(uint32_t /*part*/, const Transformation& /*transformation*/,
						   float3_p /*p*/, float3_p /*wi*/, float /*area*/,
						   bool /*two_sided*/, bool total_sphere,
						   Node_stack& /*node_stack*/) const {
	if (total_sphere) {
		return 1.f / (4.f * math::Pi);
	} else {
		return 1.f / (2.f * math::Pi);
	}
}

void Infinite_sphere::sample(uint32_t /*part*/, const Transformation& transformation,
							 float3_p /*p*/, float2 uv, float /*area*/, bool /*two_sided*/,
							 Sample& sample) const {
	const float phi   = (uv.v[0] - 0.5f) * (2.f * math::Pi);
	const float theta = uv.v[1] * math::Pi;

	const float sin_phi   = std::sin(phi);
	const float cos_phi   = std::cos(phi);
	const float sin_theta = std::sin(theta);
	const float cos_theta = std::cos(theta);

	const float3 dir(sin_phi * sin_theta, cos_theta, cos_phi * sin_theta);

	sample.wi = math::transform_vector(dir, transformation.rotation);
	sample.uv = uv;
	sample.t  = Ray_max_t;
	// sin_theta because of the uv weight
	sample.pdf = 1.f / ((4.f * math::Pi) * sin_theta);
}

float Infinite_sphere::pdf_uv(uint32_t /*part*/, const Transformation& transformation,
							  float3_p /*p*/, float3_p wi, float /*area*/, bool /*two_sided*/,
							  float2& uv) const {
	float3 xyz = math::transform_vector_transposed(wi, transformation.rotation);
	xyz = math::normalized(xyz);
	uv.v[0] = std::atan2(xyz.v[0], xyz.v[2]) * (math::Pi_inv * 0.5f) + 0.5f;
	uv.v[1] = std::acos(xyz.v[1]) * math::Pi_inv;

	// sin_theta because of the uv weight
	const float sin_theta = std::sqrt(1.f - xyz.v[1] * xyz.v[1]);

	return 1.f / ((4.f * math::Pi) * sin_theta);
}

float Infinite_sphere::uv_weight(float2 uv) const {
	float sin_theta = std::sin(uv.v[1] * math::Pi);

	return sin_theta;
}

float Infinite_sphere::area(uint32_t /*part*/, float3_p /*scale*/) const {
	return 4.f * math::Pi;
}

bool Infinite_sphere::is_finite() const {
	return false;
}

size_t Infinite_sphere::num_bytes() const {
	return sizeof(*this);
}

}}
