#include "canopy.hpp"
#include "shape_sample.hpp"
#include "geometry/shape_intersection.hpp"
#include "scene/scene_ray.inl"
#include "scene/entity/composed_transformation.hpp"
#include "sampler/sampler.hpp"
#include "base/math/mapping.inl"
#include "base/math/sampling/sampling.inl"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/bounding/aabb.inl"

#include <iostream>

namespace scene { namespace shape {

Canopy::Canopy() {
	aabb_.set_min_max(math::float3_identity, math::float3_identity);
}

bool Canopy::intersect(const Transformation& transformation, Ray& ray,
					   Node_stack& /*node_stack*/, Intersection& intersection) const {
	if (ray.max_t >= 1000000.f) {
		if (math::dot(ray.direction, transformation.rotation.v3.z) < 0.f) {
			return false;
		}

		intersection.epsilon = 5e-4f;

		intersection.p = ray.point(1000000.f);
		intersection.t = transformation.rotation.v3.x;
		intersection.b = transformation.rotation.v3.y;

		float3 n = -ray.direction;
		intersection.n = n;
		intersection.geo_n = n;
		intersection.part = 0;

		// paraboloid, so doesn't match hemispherical camera
		float3 xyz = math::transform_vector_transposed(ray.direction,
													   transformation.rotation);
		xyz = math::normalized(xyz);
		float2 disk = math::hemisphere_to_disk_equidistant(xyz);
		intersection.uv.x = 0.5f * disk.x + 0.5f;
		intersection.uv.y = 0.5f * disk.y + 0.5f;

		ray.max_t = 1000000.f;
		return true;
	}

	return false;
}

bool Canopy::intersect_p(const Transformation& /*transformation*/,
						 const Ray& /*ray*/, Node_stack& /*node_stack*/) const {
	// Implementation for this is not really needed, so just skip it
	return false;
}

float Canopy::opacity(const Transformation& /*transformation*/,
					  const Ray& /*ray*/, const material::Materials& /*materials*/,
					  Worker& /*worker*/, Sampler_filter /*filter*/) const {
	// Implementation for this is not really needed, so just skip it
	return 0.f;
}

void Canopy::sample(uint32_t /*part*/, const Transformation& transformation,
					float3_p /*p*/, float3_p /*n*/, float /*area*/,
					bool /*two_sided*/, sampler::Sampler& sampler,
					Node_stack& /*node_stack*/, Sample& sample) const {
	float2 uv = sampler.generate_sample_2D();
	float3 dir = math::sample_oriented_hemisphere_uniform(uv, transformation.rotation);

	sample.wi = dir;

	float3 xyz = math::transform_vector_transposed(dir, transformation.rotation);
	xyz = math::normalized(xyz);
	float2 disk = math::hemisphere_to_disk_equidistant(xyz);
	sample.uv.x = 0.5f * disk.x + 0.5f;
	sample.uv.y = 0.5f * disk.y + 0.5f;

	sample.t   = 1000000.f;
	sample.pdf = 1.f / (2.f * math::Pi);
}

void Canopy::sample(uint32_t /*part*/, const Transformation& transformation,
					float3_p /*p*/, float /*area*/, bool /*two_sided*/,
					sampler::Sampler& sampler, Node_stack& /*node_stack*/,
					Sample& sample) const {
	float2 uv = sampler.generate_sample_2D();
	float3 dir = math::sample_oriented_hemisphere_uniform(uv, transformation.rotation);

	sample.wi = dir;

	float3 xyz = math::transform_vector_transposed(dir, transformation.rotation);
	xyz = math::normalized(xyz);
	float2 disk = math::hemisphere_to_disk_equidistant(xyz);
	sample.uv.x = 0.5f * disk.x + 0.5f;
	sample.uv.y = 0.5f * disk.y + 0.5f;

	sample.t   = 1000000.f;
	sample.pdf = 1.f / (2.f * math::Pi);
}

void Canopy::sample(uint32_t /*part*/, const Transformation& transformation,
					float3_p /*p*/, float2 uv, float /*area*/, Sample& sample) const {
	float2 disk(2.f * uv.x - 1.f, 2.f * uv.y - 1.f);

	float z = math::dot(disk, disk);
	if (z > 1.f) {
		sample.pdf = 0.f;
		return;
	}

	float3 dir = math::disk_to_hemisphere_equidistant(disk);

	sample.wi  = math::transform_vector(dir, transformation.rotation);
	sample.uv  = uv;
	sample.t   = 1000000.f;
	sample.pdf = 1.f / (2.f * math::Pi);
}

void Canopy::sample(uint32_t /*part*/, const Transformation& /*transformation*/,
					float3_p /*p*/, float3_p /*wi*/, float /*area*/, Sample& /*sample*/) const {
	// TODO
	std::cout << "Canopy::sample() not implemented!" << std::endl;
}

float Canopy::pdf(uint32_t /*part*/, const Transformation& /*transformation*/,
				  float3_p /*p*/, float3_p /*wi*/, float /*area*/, bool /*two_sided*/,
				  bool /*total_sphere*/, Node_stack& /*node_stack*/) const {
	return 1.f / (2.f * math::Pi);
}

float Canopy::area(uint32_t /*part*/, float3_p /*scale*/) const {
	return 2.f * math::Pi;
}

float Canopy::uv_weight(float2 /*uv*/) const {
	return 1.f;
}

bool Canopy::is_finite() const {
	return false;
}

size_t Canopy::num_bytes() const {
	return sizeof(*this);
}

}}
