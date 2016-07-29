#include "plane.hpp"
#include "shape_sample.hpp"
#include "geometry/shape_intersection.hpp"
#include "scene/scene_worker.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "base/math/vector.inl"
#include "base/math/ray.inl"
#include "base/math/bounding/aabb.inl"

namespace scene { namespace shape {

Plane::Plane() {
	aabb_.set_min_max(math::float3_identity, math::float3_identity);
}

bool Plane::intersect(const Transformation& transformation, math::Oray& ray,
					  Node_stack& /*node_stack*/, Intersection& intersection) const {
	const float3& normal = transformation.rotation.v3.z;
	float d = -math::dot(normal, transformation.position);
	float denom = math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) + d;
	float t = -(numer / denom);

	if (t > ray.min_t && t < ray.max_t) {
		intersection.epsilon = 5e-4f * t;

		intersection.p = ray.point(t);
		intersection.t = -transformation.rotation.v3.x;
		intersection.b = -transformation.rotation.v3.y;
		intersection.n = normal;
		intersection.geo_n = normal;
		intersection.uv.x = math::dot(intersection.t, intersection.p) * transformation.scale.x;
		intersection.uv.y = math::dot(intersection.b, intersection.p) * transformation.scale.y;

		intersection.part = 0;

		ray.max_t = t;
		return true;
	}

	return false;


/*
	math::float3a vn(transformation.rotation.z);
	math::float3a vp(transformation.position);
	math::float3a vd(ray.direction);
	math::float3a vo(ray.origin);

	math::simd::Vector normal = math::simd::load_float3(vn);
	math::simd::Vector position = math::simd::load_float3(vp);
	math::simd::Vector direction = math::simd::load_float3(vd);
	math::simd::Vector origin = math::simd::load_float3(vo);

	math::simd::Vector d = math::simd::dot3(normal, position);
	math::simd::Vector denom = math::simd::dot3(normal, direction);
	math::simd::Vector numer = math::simd::dot3(normal, origin);
	numer = math::simd::sub3(numer, d);

	math::simd::Vector ttt = math::simd::div3(numer, denom);

	math::float3a tt;
	math::simd::store_float3(tt, ttt);

	float t = -tt.x;

	if (t > ray.min_t && t < ray.max_t) {
		intersection.epsilon = 5e-4f * t;

		intersection.p = ray.point(t);
		intersection.t = -transformation.rotation.x;
		intersection.b = -transformation.rotation.y;
		intersection.n = transformation.rotation.z;
		intersection.geo_n = transformation.rotation.z;
		intersection.uv.x = math::dot(intersection.t, intersection.p) * transformation.scale.x;
		intersection.uv.y = math::dot(intersection.b, intersection.p) * transformation.scale.y;

		intersection.part = 0;

		ray.max_t = t;
		return true;
	}

	return false;
*/
}

bool Plane::intersect_p(const Transformation& transformation, const math::Oray& ray,
						Node_stack& /*node_stack*/) const {
	const float3& normal = transformation.rotation.v3.z;
	float d = -math::dot(normal, transformation.position);
	float denom = math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) + d;
	float t = -(numer / denom);

	if (t > ray.min_t && t < ray.max_t) {
		return true;
	}

	return false;
}

float Plane::opacity(const Transformation& transformation, const math::Oray& ray,
					 float time, const material::Materials& materials,
					 Worker& worker, Sampler_filter filter) const {
	const float3& normal = transformation.rotation.v3.z;
	float d = -math::dot(normal, transformation.position);
	float denom = math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) + d;
	float t = -(numer / denom);

	if (t > ray.min_t && t < ray.max_t) {
		float3 p = ray.point(t);
		float2 uv(math::dot(transformation.rotation.v3.x, p),
						math::dot(transformation.rotation.v3.y, p));

		return materials[0]->opacity(uv, time, worker, filter);
	}

	return 0.f;
}

void Plane::sample(uint32_t /*part*/, const Transformation& /*transformation*/,
				   float3_p /*p*/, float3_p /*n*/, float /*area*/,
				   bool /*two_sided*/, sampler::Sampler& /*sampler*/,
				   Node_stack& /*node_stack*/, Sample& sample) const {
	sample.pdf = 0.f;
}

void Plane::sample(uint32_t /*part*/, const Transformation& /*transformation*/,
				   float3_p /*p*/, float /*area*/, bool /*two_sided*/,
				   sampler::Sampler& /*sampler*/, Node_stack& /*node_stack*/,
				   Sample& sample) const {
	sample.pdf = 0.f;
}

void Plane::sample(uint32_t /*part*/, const Transformation& /*transformation*/,
				   float3_p /*p*/, float2 /*uv*/, float /*area*/,
				   Sample& /*sample*/) const {}

void Plane::sample(uint32_t /*part*/, const Transformation& /*transformation*/,
				   float3_p /*p*/, float3_p /*wi*/, float /*area*/,
				   Sample& /*sample*/) const {}

float Plane::pdf(uint32_t /*part*/, const Transformation& /*transformation*/,
				 float3_p /*p*/, float3_p /*wi*/, float /*area*/,
				 bool /*two_sided*/, bool /*total_sphere*/,
				 Node_stack& /*node_stack*/) const {
	return 0.f;
}

float Plane::area(uint32_t /*part*/, float3_p /*scale*/) const {
	return 1.f;
}

bool Plane::is_finite() const {
	return false;
}

}}
