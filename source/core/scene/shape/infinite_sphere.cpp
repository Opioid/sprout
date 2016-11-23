#include "infinite_sphere.hpp"
#include "shape_sample.hpp"
#include "shape_intersection.hpp"
#include "scene/scene_ray.inl"
#include "scene/entity/composed_transformation.hpp"
#include "sampler/sampler.hpp"
#include "base/math/sampling/sampling.inl"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/bounding/aabb.inl"

#include <iostream>

namespace scene { namespace shape {

Infinite_sphere::Infinite_sphere() {
	aabb_.set_min_max(math::float3_identity, math::float3_identity);
}

bool Infinite_sphere::intersect(const Transformation& transformation,
								Ray& ray, Node_stack& /*node_stack*/,
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

void Infinite_sphere::sample(uint32_t /*part*/, const Transformation& transformation,
							 float3_p /*p*/, float3_p n, float /*area*/, bool /*two_sided*/,
							 sampler::Sampler& sampler, uint32_t sampler_dimension,
							 Node_stack& /*node_stack*/, Sample& sample) const {
	float3 x, y;
	math::coordinate_system(n, x, y);

	float2 uv = sampler.generate_sample_2D(sampler_dimension);
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
							 float3_p /*p*/, float /*area*/, bool /*two_sided*/,
							 sampler::Sampler& sampler, uint32_t sampler_dimension,
							 Node_stack& /*node_stack*/, Sample& sample) const {
	float2 uv = sampler.generate_sample_2D(sampler_dimension);
	float3 dir = math::sample_sphere_uniform(uv);

	sample.wi = dir;

	float3 xyz = math::transform_vector_transposed(dir, transformation.rotation);
	xyz = math::normalized(xyz);
	sample.uv.x = std::atan2(xyz.x, xyz.z) * math::Pi_inv * 0.5f + 0.5f;
	sample.uv.y = std::acos(xyz.y) * math::Pi_inv;

	sample.t   = 1000000.f;
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
							 float3_p /*p*/, float2 uv, float /*area*/, Sample& sample) const {
	float phi   = (uv.x - 0.5f) * 2.f * math::Pi;
	float theta = uv.y * math::Pi;

	float sin_phi   = std::sin(phi);
	float cos_phi   = std::cos(phi);
	float sin_theta = std::sin(theta);
	float cos_theta = std::cos(theta);

	float3 dir(sin_phi * sin_theta, cos_theta, cos_phi * sin_theta);

	sample.wi = math::transform_vector(dir, transformation.rotation);
	sample.uv = uv;
	sample.t  = 1000000.f;
	// sin_theta because of the uv weight
	sample.pdf = 1.f / (4.f * math::Pi * sin_theta);
}

float Infinite_sphere::pdf_uv(uint32_t /*part*/, const Transformation& transformation,
							  float3_p /*p*/, float3_p wi, float /*area*/, float2& uv) const {
	float3 xyz = math::transform_vector_transposed(wi, transformation.rotation);
	xyz = math::normalized(xyz);
	uv.x = std::atan2(xyz.x, xyz.z) * math::Pi_inv * 0.5f + 0.5f;
	uv.y = std::acos(xyz.y) * math::Pi_inv;

	// sin_theta because of the uv weight
	float sin_theta = std::sqrt(1.f - xyz.y * xyz.y);

	return 1.f / (4.f * math::Pi * sin_theta);
}

float Infinite_sphere::uv_weight(float2 uv) const {
	float sin_theta = std::sin(uv.y * math::Pi);

	if (0.f == sin_theta) {
		// this case never seemed to be an issue?!
		return 0.f;
	}

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
