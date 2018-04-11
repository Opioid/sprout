#include "infinite_sphere.hpp"
#include "shape_sample.hpp"
#include "shape_intersection.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/entity/composed_transformation.hpp"
#include "sampler/sampler.hpp"
#include "base/math/sincos.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/sampling/sampling.hpp"

#include "shape_test.hpp"
#include "base/debug/assert.hpp"

namespace scene::shape {

Infinite_sphere::Infinite_sphere() {
	aabb_.set_min_max(float3::identity(), float3::identity());
}

bool Infinite_sphere::intersect(const Transformation& transformation,
								Ray& ray, Node_stack& /*node_stack*/,
								Intersection& intersection) const {
	if (ray.max_t > Ray_max_t) {
		intersection.epsilon = 5e-4f;

		// This is nonsense
		intersection.t = transformation.rotation.r[0];
		intersection.b = transformation.rotation.r[1];

		float3 xyz = math::transform_vector_transposed(ray.direction, transformation.rotation);
		xyz = math::normalize(xyz);
		intersection.uv[0] = std::atan2(xyz[0], xyz[2]) * (math::Pi_inv * 0.5f) + 0.5f;
		intersection.uv[1] = std::acos(xyz[1]) * math::Pi_inv;

		intersection.p = ray.point(Ray_max_t);
		const float3 n = -ray.direction;
		intersection.n = n;
		intersection.geo_n = n;
		intersection.part = 0;

		ray.max_t = Ray_max_t;

		SOFT_ASSERT(testing::check(intersection, transformation, ray));

		return true;
	}

	return false;
}

bool Infinite_sphere::intersect(const Transformation& /*transformation*/,
								Ray& /*ray*/, Node_stack& /*node_stack*/,
								float& /*epsilon*/, bool& /*inside*/) const {
	return false;
}


bool Infinite_sphere::intersect_p(const Transformation& /*transformation*/,
								  const Ray& /*ray*/, Node_stack& /*node_stack*/) const {
	// Implementation for this is not really needed, so just skip it
	return false;
}

float Infinite_sphere::opacity(const Transformation& /*transformation*/,
							   const Ray& /*ray*/, const Materials& /*materials*/,
							   Sampler_filter /*filter*/,const Worker& /*worker*/) const {
	// Implementation for this is not really needed, so just skip it
	return 0.f;
}

float3 Infinite_sphere::thin_absorption(const Transformation& /*transformation*/,
										const Ray& /*ray*/,
										const Materials& /*materials*/,
										Sampler_filter /*filter*/, const Worker& /*worker*/) const {
	// Implementation for this is not really needed, so just skip it
	return float3(0.f);
}

bool Infinite_sphere::sample(uint32_t /*part*/, const Transformation& transformation,
							 const float3& /*p*/, const float3& n,
							 float /*area*/, bool /*two_sided*/,
							 sampler::Sampler& sampler, uint32_t sampler_dimension,
							 Node_stack& /*node_stack*/, Sample& sample) const {
	float3 x, y;
	math::orthonormal_basis(n, x, y);

	const float2 uv = sampler.generate_sample_2D(sampler_dimension);
	const float3 dir = math::sample_oriented_hemisphere_uniform(uv, x, y, n);

	sample.wi = dir;

	float3 xyz = math::transform_vector_transposed(dir, transformation.rotation);
	xyz = math::normalize(xyz);
	sample.uv[0] = std::atan2(xyz[0], xyz[2]) * (math::Pi_inv * 0.5f) + 0.5f;
	sample.uv[1] = std::acos(xyz[1]) * math::Pi_inv;

	sample.pdf = 1.f / (2.f * math::Pi);
	sample.t   = Ray_max_t;
	sample.epsilon = 5e-4f;

	SOFT_ASSERT(testing::check(sample));

	return true;
}

bool Infinite_sphere::sample(uint32_t /*part*/, const Transformation& transformation,
							 const float3& /*p*/, float /*area*/, bool /*two_sided*/,
							 sampler::Sampler& sampler, uint32_t sampler_dimension,
							 Node_stack& /*node_stack*/, Sample& sample) const {
	const float2 uv = sampler.generate_sample_2D(sampler_dimension);
	const float3 dir = math::sample_sphere_uniform(uv);

	sample.wi = dir;

	float3 xyz = math::transform_vector_transposed(dir, transformation.rotation);
	xyz = math::normalize(xyz);
	sample.uv[0] = std::atan2(xyz[0], xyz[2]) * (math::Pi_inv * 0.5f) + 0.5f;
	sample.uv[1] = std::acos(xyz[1]) * math::Pi_inv;

	sample.pdf = 1.f / (4.f * math::Pi);
	sample.t   = Ray_max_t;
	sample.epsilon = 5e-4f;

	SOFT_ASSERT(testing::check(sample));

	return true;
}

float Infinite_sphere::pdf(const Ray& /*ray*/, const shape::Intersection& /*intersection*/,
						   const Transformation& /*transformation*/,
						   float /*area*/, bool /*two_sided*/, bool total_sphere) const {
	if (total_sphere) {
		return 1.f / (4.f * math::Pi);
	} else {
		return 1.f / (2.f * math::Pi);
	}
}

bool Infinite_sphere::sample(uint32_t /*part*/, const Transformation& transformation,
							 const float3& /*p*/, float2 uv, float /*area*/, bool /*two_sided*/,
							 Sample& sample) const {
	const float phi   = (uv[0] - 0.5f) * (2.f * math::Pi);
	const float theta = uv[1] * math::Pi;

//	const float sin_phi   = std::sin(phi);
//	const float cos_phi   = std::cos(phi);
//	const float sin_theta = std::sin(theta);
//	const float cos_theta = std::cos(theta);
	float sin_phi;
	float cos_phi;
	math::sincos(phi, sin_phi, cos_phi);
	float sin_theta;
	float cos_theta;
	math::sincos(theta, sin_theta, cos_theta);

	const float3 dir(sin_phi * sin_theta, cos_theta, cos_phi * sin_theta);

	sample.wi = math::transform_vector(dir, transformation.rotation);
	sample.uv = uv;
	sample.t  = Ray_max_t;
	// sin_theta because of the uv weight
	sample.pdf = 1.f / ((4.f * math::Pi) * sin_theta);

	SOFT_ASSERT(testing::check(sample, uv));

	return true;
}

float Infinite_sphere::pdf_uv(const Ray& /*ray*/, const Intersection& intersection,
							  const Transformation& /*transformation*/,
							  float /*area*/, bool /*two_sided*/) const {
//	float3 xyz = math::transform_vector_transposed(wi, transformation.rotation);
//	xyz = math::normalize(xyz);
//	uv[0] = std::atan2(xyz[0], xyz[2]) * (math::Pi_inv * 0.5f) + 0.5f;
//	uv[1] = std::acos(xyz[1]) * math::Pi_inv;

//	// sin_theta because of the uv weight
//	const float sin_theta = std::sqrt(1.f - xyz[1] * xyz[1]);

	const float sin_theta = std::sin(intersection.uv[1] * math::Pi);

	return 1.f / ((4.f * math::Pi) * sin_theta);
}

float Infinite_sphere::uv_weight(float2 uv) const {
	const float sin_theta = std::sin(uv[1] * math::Pi);

	return sin_theta;
}

float Infinite_sphere::area(uint32_t /*part*/, const float3& /*scale*/) const {
	return 4.f * math::Pi;
}

bool Infinite_sphere::is_finite() const {
	return false;
}

size_t Infinite_sphere::num_bytes() const {
	return sizeof(*this);
}

}
