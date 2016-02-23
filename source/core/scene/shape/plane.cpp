#include "plane.hpp"
#include "shape_sample.hpp"
#include "geometry/shape_intersection.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "base/math/vector.inl"
#include "base/math/ray.inl"
#include "base/math/bounding/aabb.inl"

#include "base/math/simd/simd_vector.inl"

namespace scene { namespace shape {

Plane::Plane() {
	aabb_.set_min_max(math::vec3_identity, math::vec3_identity);
}

bool Plane::intersect(const entity::Composed_transformation& transformation, math::Oray& ray,
					  Node_stack& /*node_stack*/, Intersection& intersection) const {
	const math::vec3& normal = transformation.rotation.z3;
	float d = -math::dot(normal, transformation.position);
	float denom = math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) + d;
	float t = -(numer / denom);

	if (t > ray.min_t && t < ray.max_t) {
		intersection.epsilon = 5e-4f * t;

		intersection.p = ray.point(t);
		intersection.t = -transformation.rotation.x3;
		intersection.b = -transformation.rotation.y3;
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
	math::vec3a vn(transformation.rotation.z);
	math::vec3a vp(transformation.position);
	math::vec3a vd(ray.direction);
	math::vec3a vo(ray.origin);

	math::simd::Vector normal = math::simd::load_vec3(vn);
	math::simd::Vector position = math::simd::load_vec3(vp);
	math::simd::Vector direction = math::simd::load_vec3(vd);
	math::simd::Vector origin = math::simd::load_vec3(vo);

	math::simd::Vector d = math::simd::dot3(normal, position);
	math::simd::Vector denom = math::simd::dot3(normal, direction);
	math::simd::Vector numer = math::simd::dot3(normal, origin);
	numer = math::simd::sub3(numer, d);

	math::simd::Vector ttt = math::simd::div3(numer, denom);

	math::vec3a tt;
	math::simd::store_vec3(tt, ttt);

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

bool Plane::intersect_p(const entity::Composed_transformation& transformation, const math::Oray& ray,
						Node_stack& /*node_stack*/) const {
	const math::vec3& normal = transformation.rotation.z3;
	float d = -math::dot(normal, transformation.position);
	float denom = math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) + d;
	float t = -(numer / denom);

	if (t > ray.min_t && t < ray.max_t) {
		return true;
	}

	return false;
}

float Plane::opacity(const entity::Composed_transformation& transformation, const math::Oray& ray, float time,
					 Node_stack& /*node_stack*/, const material::Materials& materials,
					 const image::texture::sampler::Sampler_2D& sampler) const {
	const math::vec3& normal = transformation.rotation.z3;
	float d = -math::dot(normal, transformation.position);
	float denom = math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) + d;
	float t = -(numer / denom);

	if (t > ray.min_t && t < ray.max_t) {
		math::vec3 p = ray.point(t);
		math::float2 uv(math::dot(transformation.rotation.x3, p), math::dot(transformation.rotation.y3, p));

		return materials[0]->opacity(uv, time, sampler);
	}

	return 0.f;
}

void Plane::sample(uint32_t /*part*/, const entity::Composed_transformation& /*transformation*/, float /*area*/,
				   const math::vec3& /*p*/, const math::vec3& /*n*/, bool /*two_sided*/,
				   sampler::Sampler& /*sampler*/, Node_stack& /*node_stack*/, Sample& sample) const {
	sample.pdf = 0.f;
}

void Plane::sample(uint32_t /*part*/, const entity::Composed_transformation& /*transformation*/, float /*area*/,
				   const math::vec3& /*p*/, bool /*two_sided*/,
				   sampler::Sampler& /*sampler*/, Node_stack& /*node_stack*/, Sample& sample) const {
	sample.pdf = 0.f;
}

void Plane::sample(uint32_t /*part*/, const entity::Composed_transformation& /*transformation*/, float /*area*/,
				   const math::vec3& /*p*/, math::float2 /*uv*/, Sample& /*sample*/) const {}

void Plane::sample(uint32_t /*part*/, const entity::Composed_transformation& /*transformation*/, float /*area*/,
				   const math::vec3& /*p*/, const math::vec3& /*wi*/, Sample& /*sample*/) const {}

float Plane::pdf(uint32_t /*part*/, const entity::Composed_transformation& /*transformation*/, float /*area*/,
				 const math::vec3& /*p*/, const math::vec3& /*wi*/, bool /*two_sided*/, bool /*total_sphere*/,
				 Node_stack& /*node_stack*/) const {
	return 0.f;
}

float Plane::area(uint32_t /*part*/, const math::vec3& /*scale*/) const {
	return 1.f;
}

bool Plane::is_finite() const {
	return false;
}

}}
