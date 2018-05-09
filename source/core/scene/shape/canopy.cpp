#include "canopy.hpp"
#include "shape_sample.hpp"
#include "shape_intersection.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/entity/composed_transformation.hpp"
#include "sampler/sampler.hpp"
#include "base/math/mapping.inl"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/sampling/sampling.hpp"

#include "shape_test.hpp"
#include "base/debug/assert.hpp"

namespace scene::shape {

Canopy::Canopy() {
	aabb_.set_min_max(float3::identity(), float3::identity());
}

bool Canopy::intersect(Ray& ray, Transformation const& transformation,
					   Node_stack& /*node_stack*/, Intersection& intersection) const {
	if (ray.max_t >= Ray_max_t) {
		if (math::dot(ray.direction, transformation.rotation.r[2]) < 0.f) {
			return false;
		}

		intersection.epsilon = 5e-4f;

		intersection.p = ray.point(Ray_max_t);
		intersection.t = transformation.rotation.r[0];
		intersection.b = transformation.rotation.r[1];

		float3 n = -ray.direction;
		intersection.n = n;
		intersection.geo_n = n;
		intersection.part = 0;

		// paraboloid, so doesn't match hemispherical camera
		float3 xyz = math::transform_vector_transposed(ray.direction, transformation.rotation);
		xyz = math::normalize(xyz);
		float2 disk = math::hemisphere_to_disk_equidistant(xyz);
		intersection.uv[0] = 0.5f * disk[0] + 0.5f;
		intersection.uv[1] = 0.5f * disk[1] + 0.5f;

		ray.max_t = Ray_max_t;

		SOFT_ASSERT(testing::check(intersection, transformation, ray));

		return true;
	}

	return false;
}

bool Canopy::intersect_fast(Ray& ray, Transformation const& transformation,
							Node_stack& /*node_stack*/, Intersection& intersection) const {
	if (ray.max_t >= Ray_max_t) {
		if (math::dot(ray.direction, transformation.rotation.r[2]) < 0.f) {
			return false;
		}

		intersection.epsilon = 5e-4f;

		intersection.p = ray.point(Ray_max_t);

		float3 n = -ray.direction;
		intersection.geo_n = n;
		intersection.part = 0;

		// paraboloid, so doesn't match hemispherical camera
		float3 xyz = math::transform_vector_transposed(ray.direction, transformation.rotation);
		xyz = math::normalize(xyz);
		float2 disk = math::hemisphere_to_disk_equidistant(xyz);
		intersection.uv[0] = 0.5f * disk[0] + 0.5f;
		intersection.uv[1] = 0.5f * disk[1] + 0.5f;

		ray.max_t = Ray_max_t;

		SOFT_ASSERT(testing::check(intersection, transformation, ray));

		return true;
	}

	return false;
}

bool Canopy::intersect(Ray& ray, Transformation const& transformation,
					   Node_stack& /*node_stack*/, float& epsilon) const {
	if (ray.max_t >= Ray_max_t) {
		if (math::dot(ray.direction, transformation.rotation.r[2]) < 0.f) {
			return false;
		}

		ray.max_t = Ray_max_t;
		epsilon = 5e-4f;

		return true;
	}

	return false;
}

bool Canopy::intersect_p(Ray const& /*ray*/, Transformation const& /*transformation*/,
						 Node_stack& /*node_stack*/) const {
	// Implementation for this is not really needed, so just skip it
	return false;
}

float Canopy::opacity(Ray const& /*ray*/, Transformation const& /*transformation*/,
					  const Materials& /*materials*/, Sampler_filter /*filter*/,
					  Worker const& /*worker*/) const {
	// Implementation for this is not really needed, so just skip it
	return 0.f;
}

float3 Canopy::thin_absorption(Ray const& /*ray*/, Transformation const& /*transformation*/,
							   const Materials& /*materials*/, Sampler_filter /*filter*/,
							   Worker const& /*worker*/) const {
	// Implementation for this is not really needed, so just skip it
	return float3(0.f);
}

bool Canopy::sample(uint32_t part, f_float3 p, f_float3 /*n*/,
					Transformation const& transformation, float area, bool two_sided,
					sampler::Sampler& sampler, uint32_t sampler_dimension,
					Node_stack& node_stack, Sample& sample) const {
	return Canopy::sample(part, p, transformation, area, two_sided,
						  sampler, sampler_dimension, node_stack, sample);
}

bool Canopy::sample(uint32_t /*part*/, f_float3 /*p*/, Transformation const& transformation,
					float /*area*/, bool /*two_sided*/,
					sampler::Sampler& sampler, uint32_t sampler_dimension,
					Node_stack& /*node_stack*/, Sample& sample) const {
	float2 const uv = sampler.generate_sample_2D(sampler_dimension);
	float3 const dir = math::sample_oriented_hemisphere_uniform(uv, transformation.rotation);

	sample.wi = dir;

	float3 xyz = math::transform_vector_transposed(dir, transformation.rotation);
	xyz = math::normalize(xyz);
	float2 const disk = math::hemisphere_to_disk_equidistant(xyz);
	sample.uv[0] = 0.5f * disk[0] + 0.5f;
	sample.uv[1] = 0.5f * disk[1] + 0.5f;
	sample.pdf = 1.f / (2.f * math::Pi);
	sample.t   = Ray_max_t;
	sample.epsilon = 5e-4f;

	SOFT_ASSERT(testing::check(sample));

	return true;
}

float Canopy::pdf(Ray const& /*ray*/, const shape::Intersection& /*intersection*/,
				  Transformation const& /*transformation*/,
				  float /*area*/, bool /*two_sided*/, bool /*total_sphere*/) const {
	return 1.f / (2.f * math::Pi);
}

bool Canopy::sample(uint32_t /*part*/, f_float3 /*p*/, float2 uv,
					Transformation const& transformation,
					float /*area*/, bool /*two_sided*/, Sample& sample) const {
	float2 const disk(2.f * uv[0] - 1.f, 2.f * uv[1] - 1.f);

	float const z = math::dot(disk, disk);
	if (z > 1.f) {
		sample.pdf = 0.f;
		return false;
	}

	float3 const dir = math::disk_to_hemisphere_equidistant(disk);

	sample.wi = math::transform_vector(dir, transformation.rotation);
	sample.uv = uv;
	sample.t  = Ray_max_t;
	sample.pdf = 1.f / (2.f * math::Pi);
	sample.epsilon = 5e-4f;

	return true;
}

float Canopy::pdf_uv(Ray const& /*ray*/, Intersection const& /*intersection*/,
					 Transformation const& /*transformation*/,
					 float /*area*/, bool /*two_sided*/) const {
	return 1.f / (2.f * math::Pi);
}

float Canopy::uv_weight(float2 uv) const {
	float2 const disk(2.f * uv[0] - 1.f, 2.f * uv[1] - 1.f);

	float const z = math::dot(disk, disk);
	if (z > 1.f) {
		return 0.f;
	}

	return 1.f;
}

float Canopy::area(uint32_t /*part*/, f_float3 /*scale*/) const {
	return 2.f * math::Pi;
}

bool Canopy::is_finite() const {
	return false;
}

size_t Canopy::num_bytes() const {
	return sizeof(*this);
}

}
