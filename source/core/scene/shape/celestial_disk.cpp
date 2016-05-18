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
	aabb_.set_min_max(math::float3_identity, math::float3_identity);
}

bool Celestial_disk::intersect(const Entity_transformation& transformation, math::Oray& ray,
							   Node_stack& /*node_stack*/, Intersection& intersection) const {
	const math::float3& v = transformation.rotation.z3;
	float b = math::dot(v, ray.direction);

	if (b > 0.f) {
		return false;
	}

	float radius = math::degrees_to_radians(transformation.scale.x);
	float det = (b * b) - math::dot(v, v) + (radius * radius);
//	float det = -b - std::cos(radius);

	if (det > 0.f && ray.max_t >= 1000000.f) {
		intersection.epsilon = 5e-4f;

		intersection.p = ray.point(999999.9f);
		intersection.t = transformation.rotation.x3;
		intersection.b = transformation.rotation.y3;
		intersection.n = transformation.rotation.z3;
		intersection.geo_n = transformation.rotation.z3;
		intersection.part = 0;

		ray.max_t = 999999.9f;
		return true;
	}

	return false;
}

bool Celestial_disk::intersect_p(const Entity_transformation& /*transformation*/,
								 const math::Oray& /*ray*/, Node_stack& /*node_stack*/) const {
	// Implementation for this is not really needed, so just skip it
	return false;
}

float Celestial_disk::opacity(const Entity_transformation& /*transformation*/,
							  const math::Oray& /*ray*/, float /*time*/,
							  const material::Materials& /*materials*/,
							  Worker& /*worker*/, Sampler_filter /*filter*/) const {
	// Implementation for this is not really needed, so just skip it
	return 0.f;
}

void Celestial_disk::sample(uint32_t part, const Entity_transformation& transformation,
							float area, const math::float3& p, const math::float3& /*n*/,
							bool two_sided, sampler::Sampler& sampler,
							Node_stack& node_stack, Sample& sample) const {
	Celestial_disk::sample(part, transformation, area, p, two_sided, sampler, node_stack, sample);
}

void Celestial_disk::sample(uint32_t /*part*/, const Entity_transformation& transformation,
							float area, const math::float3& /*p*/, bool /*two_sided*/,
							sampler::Sampler& sampler, Node_stack& /*node_stack*/,
							Sample& sample) const {
	math::float2 r2 = sampler.generate_sample_2D();
	math::float2 xy = math::sample_disk_concentric(r2);

	math::float3 ls = math::float3(xy, 0.f);
	float radius = math::degrees_to_radians(transformation.scale.x);
	math::float3 ws = radius * math::transform_vector(ls, transformation.rotation);

	sample.wi = math::normalized(ws - transformation.rotation.z3);
	sample.t = 1000000.f;
	sample.pdf = 1.f / area;
}

void Celestial_disk::sample(uint32_t /*part*/, const Entity_transformation& /*transformation*/,
							float /*area*/, const math::float3& /*p*/,
							math::float2 /*uv*/, Sample& /*sample*/) const {}

void Celestial_disk::sample(uint32_t /*part*/, const Entity_transformation& /*transformation*/,
							float /*area*/, const math::float3& /*p*/,
							const math::float3& /*wi*/, Sample& /*sample*/) const {}

float Celestial_disk::pdf(uint32_t /*part*/, const Entity_transformation& /*transformation*/,
						  float area, const math::float3& /*p*/, const math::float3& /*wi*/,
						  bool /*two_sided*/, bool /*total_sphere*/,
						  Node_stack& /*node_stack*/) const {
	return 1.f / area;
}

float Celestial_disk::area(uint32_t /*part*/, const math::float3& scale) const {
	float radius = math::degrees_to_radians(scale.x);
	return math::Pi * radius * radius;
}

bool Celestial_disk::is_finite() const {
	return false;
}

}}

