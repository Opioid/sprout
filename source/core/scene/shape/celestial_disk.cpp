#include "celestial_disk.hpp"
#include "shape_sample.hpp"
#include "geometry/shape_intersection.hpp"
#include "scene/scene_ray.inl"
#include "scene/entity/composed_transformation.hpp"
#include "sampler/sampler.hpp"
#include "base/math/sampling/sampling.inl"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/bounding/aabb.inl"

namespace scene { namespace shape {

Celestial_disk::Celestial_disk() {
	aabb_.set_min_max(math::float3_identity, math::float3_identity);
}

bool Celestial_disk::intersect(const Transformation& transformation, Ray& ray,
							   Node_stack& /*node_stack*/, Intersection& intersection) const {
	const float3& v = transformation.rotation.v3.z;
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
		intersection.t = transformation.rotation.v3.x;
		intersection.b = transformation.rotation.v3.y;
		intersection.n = transformation.rotation.v3.z;
		intersection.geo_n = transformation.rotation.v3.z;
		intersection.part = 0;

		ray.max_t = 999999.9f;
		return true;
	}

	return false;
}

bool Celestial_disk::intersect_p(const Transformation& /*transformation*/,
								 const Ray& /*ray*/, Node_stack& /*node_stack*/) const {
	// Implementation for this is not really needed, so just skip it
	return false;
}

float Celestial_disk::opacity(const Transformation& /*transformation*/,
							  const Ray& /*ray*/, const material::Materials& /*materials*/,
							  Worker& /*worker*/, Sampler_filter /*filter*/) const {
	// Implementation for this is not really needed, so just skip it
	return 0.f;
}

void Celestial_disk::sample(uint32_t part, const Transformation& transformation,
							float3_p p, float3_p /*n*/, float area,
							bool two_sided, sampler::Sampler& sampler,
							Node_stack& node_stack, Sample& sample) const {
	Celestial_disk::sample(part, transformation, p, area, two_sided, sampler, node_stack, sample);
}

void Celestial_disk::sample(uint32_t /*part*/, const Transformation& transformation,
							float3_p /*p*/, float area, bool /*two_sided*/,
							sampler::Sampler& sampler, Node_stack& /*node_stack*/,
							Sample& sample) const {
	float2 r2 = sampler.generate_sample_2D();
	float2 xy = math::sample_disk_concentric(r2);

	float3 ls = float3(xy, 0.f);
	float radius = math::degrees_to_radians(transformation.scale.x);
	float3 ws = radius * math::transform_vector(ls, transformation.rotation);

	sample.wi = math::normalized(ws - transformation.rotation.v3.z);
	sample.t = 1000000.f;
	sample.pdf = 1.f / area;
}

void Celestial_disk::sample(uint32_t /*part*/, const Transformation& /*transformation*/,
							float3_p /*p*/, float2 /*uv*/, float /*area*/,
							Sample& /*sample*/) const {}

void Celestial_disk::sample(uint32_t /*part*/, const Transformation& /*transformation*/,
							float3_p/*p*/, float3_p /*wi*/,
							float /*area*/, Sample& /*sample*/) const {}

float Celestial_disk::pdf(uint32_t /*part*/, const Transformation& /*transformation*/,
						  float3_p /*p*/, float3_p /*wi*/, float area,
						  bool /*two_sided*/, bool /*total_sphere*/,
						  Node_stack& /*node_stack*/) const {
	return 1.f / area;
}

float Celestial_disk::area(uint32_t /*part*/, float3_p scale) const {
	float radius = math::degrees_to_radians(scale.x);
	return math::Pi * radius * radius;
}

bool Celestial_disk::is_finite() const {
	return false;
}

size_t Celestial_disk::num_bytes() const {
	return sizeof(*this);
}

}}

