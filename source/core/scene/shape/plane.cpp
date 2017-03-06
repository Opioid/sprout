#include "plane.hpp"
#include "shape_sample.hpp"
#include "shape_intersection.hpp"
#include "scene/scene_ray.inl"
#include "scene/scene_worker.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"

namespace scene { namespace shape {

Plane::Plane() {
	aabb_.set_min_max(float3::identity(), float3::identity());
}

bool Plane::intersect(const Transformation& transformation, Ray& ray,
					  Node_stack& /*node_stack*/, Intersection& intersection) const {
	float3_p normal = transformation.rotation.r[2];
	float d = math::dot(normal, transformation.position);
	float denom = -math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) - d;
	float hit_t = numer / denom;

	if (hit_t > ray.min_t && hit_t < ray.max_t) {
		intersection.epsilon = 5e-4f * hit_t;

		float3 p = ray.point(hit_t);
		float3 t = -transformation.rotation.r[0];
		float3 b = -transformation.rotation.r[1];

		intersection.p = p;
		intersection.t = t;
		intersection.b = b;
		intersection.n = normal;
		intersection.geo_n = normal;
		intersection.uv[0] = math::dot(t, p) * transformation.scale[0];
		intersection.uv[1] = math::dot(b, p) * transformation.scale[1];

		intersection.part = 0;

		ray.max_t = hit_t;
		return true;
	}

	return false;
}

bool Plane::intersect_p(const Transformation& transformation, const Ray& ray,
						Node_stack& /*node_stack*/) const {
	float3_p normal = transformation.rotation.r[2];
	float d = math::dot(normal, transformation.position);
	float denom = -math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) - d;
	float hit_t = numer / denom;

	if (hit_t > ray.min_t && hit_t < ray.max_t) {
		return true;
	}

	return false;
}

float Plane::opacity(const Transformation& transformation, const Ray& ray,
					 const material::Materials& materials,
					 Worker& worker, Sampler_filter filter) const {
	float3_p normal = transformation.rotation.r[2];
	float d = math::dot(normal, transformation.position);
	float denom = -math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) - d;
	float hit_t = numer / denom;

	if (hit_t > ray.min_t && hit_t < ray.max_t) {
		float3 p = ray.point(hit_t);
		float2 uv(math::dot(transformation.rotation.r[0], p),
				  math::dot(transformation.rotation.r[1], p));

		return materials[0]->opacity(uv, ray.time, worker, filter);
	}

	return 0.f;
}

float3 Plane::thin_absorption(const Transformation& transformation, const Ray& ray,
							  const material::Materials& materials,
							  Worker& worker, Sampler_filter filter) const {
	float3_p normal = transformation.rotation.r[2];
	float d = math::dot(normal, transformation.position);
	float denom = -math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) - d;
	float hit_t = numer / denom;

	if (hit_t > ray.min_t && hit_t < ray.max_t) {
		float3 p = ray.point(hit_t);
		float2 uv(math::dot(transformation.rotation.r[0], p),
				  math::dot(transformation.rotation.r[1], p));

		return materials[0]->thin_absorption(ray.direction, normal, uv,
											 ray.time, worker, filter);
	}

	return float3(0.f);
}

void Plane::sample(uint32_t /*part*/, const Transformation& /*transformation*/,
				   float3_p /*p*/, float3_p /*n*/, float /*area*/, bool /*two_sided*/,
				   sampler::Sampler& /*sampler*/, uint32_t /*sampler_dimension*/,
				   Node_stack& /*node_stack*/, Sample& sample) const {
	sample.pdf = 0.f;
}

void Plane::sample(uint32_t /*part*/, const Transformation& /*transformation*/,
				   float3_p /*p*/, float /*area*/, bool /*two_sided*/,
				   sampler::Sampler& /*sampler*/, uint32_t /*sampler_dimension*/,
				   Node_stack& /*node_stack*/, Sample& sample) const {
	sample.pdf = 0.f;
}

float Plane::pdf(uint32_t /*part*/, const Transformation& /*transformation*/,
				 float3_p /*p*/, float3_p /*wi*/, float /*area*/,
				 bool /*two_sided*/, bool /*total_sphere*/,
				 Node_stack& /*node_stack*/) const {
	return 0.f;
}

void Plane::sample(uint32_t /*part*/, const Transformation& /*transformation*/,
				   float3_p /*p*/, float2 /*uv*/, float /*area*/, bool /*two_sided*/,
				   Sample& /*sample*/) const {}

float Plane::pdf_uv(uint32_t /*part*/, const Transformation& /*transformation*/,
					float3_p /*p*/, float3_p /*wi*/, float /*area*/, bool /*two_sided*/,
					float2& /*uv*/) const {
	return 1.f;
}

float Plane::uv_weight(float2 /*uv*/) const {
	return 1.f;
}

float Plane::area(uint32_t /*part*/, float3_p /*scale*/) const {
	return 1.f;
}

bool Plane::is_finite() const {
	return false;
}

size_t Plane::num_bytes() const {
	return sizeof(*this);
}

}}
