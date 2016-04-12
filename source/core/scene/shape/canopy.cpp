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

#include <iostream>

namespace scene { namespace shape {

Canopy::Canopy() {
	aabb_.set_min_max(math::float3_identity, math::float3_identity);
}

bool Canopy::intersect(const entity::Composed_transformation& transformation, math::Oray& ray,
					   Node_stack& /*node_stack*/, Intersection& intersection) const {
	if (ray.max_t >= 1000000.f) {
		if (math::dot(ray.direction, transformation.rotation.z3) < 0.f) {
			return false;
		}

		intersection.epsilon = 5e-4f;

		intersection.p = ray.point(1000000.f);
		intersection.t = transformation.rotation.x3;
		intersection.b = transformation.rotation.y3;

		math::float3 n = -ray.direction;
		intersection.n = n;
		intersection.geo_n = n;
		intersection.part = 0;

		math::float3 xyz = math::normalized(math::transform_vector_transposed(ray.direction, transformation.rotation));

		intersection.uv.x =  0.5f * (xyz.x / (xyz.z + 1.f)) + 0.5f;
		intersection.uv.y = -0.5f * (xyz.y / (xyz.z + 1.f)) + 0.5f;

		ray.max_t = 1000000.f;
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
					const math::float3& /*p*/, const math::float3& /*n*/, bool /*two_sided*/,
					sampler::Sampler& sampler, Node_stack& /*node_stack*/, Sample& sample) const {
	math::float2 uv = sampler.generate_sample_2D();
	math::float3 dir = math::sample_oriented_hemisphere_uniform(uv,
																transformation.rotation.x3,
																transformation.rotation.y3,
																transformation.rotation.z3);

	sample.wi = dir;

	math::float3 xyz = math::transform_vector_transposed(dir, transformation.rotation);
	sample.uv.x =  0.5f * (xyz.x / (xyz.z + 1.f)) + 0.5f;
	sample.uv.y = -0.5f * (xyz.y / (xyz.z + 1.f)) + 0.5f;

	sample.t   = 1000000.f;
	sample.pdf = 1.f / (2.f * math::Pi);
}

void Canopy::sample(uint32_t /*part*/, const entity::Composed_transformation& transformation, float /*area*/,
					const math::float3& /*p*/, bool /*two_sided*/,
					sampler::Sampler& sampler, Node_stack& /*node_stack*/, Sample& sample) const {
	math::float2 uv = sampler.generate_sample_2D();
	math::float3 dir = math::sample_oriented_hemisphere_uniform(uv,
																transformation.rotation.x3,
																transformation.rotation.y3,
																transformation.rotation.z3);

	sample.wi = dir;

	math::float3 xyz = math::transform_vector_transposed(dir, transformation.rotation);
	sample.uv.x =  0.5f * (xyz.x / (xyz.z + 1.f)) + 0.5f;
	sample.uv.y = -0.5f * (xyz.y / (xyz.z + 1.f)) + 0.5f;

	sample.t   = 1000000.f;
	sample.pdf = 1.f / (2.f * math::Pi);
}

void Canopy::sample(uint32_t /*part*/, const entity::Composed_transformation& transformation, float /*area*/,
					const math::float3& /*p*/, math::float2 uv, Sample& sample) const {
	// TODO
	std::cout << "Canopy::sample() not implemented!" << std::endl;
}

void Canopy::sample(uint32_t /*part*/, const entity::Composed_transformation& transformation, float /*area*/,
					const math::float3& /*p*/, const math::float3& wi, Sample& sample) const {
	// TODO
	std::cout << "Canopy::sample() not implemented!" << std::endl;
}

float Canopy::pdf(uint32_t /*part*/, const entity::Composed_transformation& /*transformation*/, float /*area*/,
				  const math::float3& /*p*/, const math::float3& /*wi*/, bool /*two_sided*/, bool /*total_sphere*/,
				  Node_stack& /*node_stack*/) const {
	return 1.f / (2.f * math::Pi);
}

float Canopy::area(uint32_t /*part*/, const math::float3& /*scale*/) const {
	return 2.f * math::Pi;
}

bool Canopy::is_finite() const {
	return false;
}

}}
