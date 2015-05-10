#include "plane.hpp"
#include "geometry/shape_intersection.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "base/math/vector.inl"
#include "base/math/ray.inl"
#include "base/math/bounding/aabb.inl"

namespace scene { namespace shape {

Plane::Plane() {
	aabb_.set_min_max(math::float3::identity, math::float3::identity);
}

bool Plane::intersect(const Composed_transformation& transformation, const math::Oray& ray,
					  const math::float2& /*bounds*/, Node_stack& /*node_stack*/,
					  Intersection& intersection, float& hit_t) const {
	const math::float3& normal = transformation.rotation.z;
	float d = -math::dot(normal, transformation.position);
	float denom = math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) + d;
	float t = -(numer / denom);

	if (t > ray.min_t && t < ray.max_t) {
		intersection.epsilon = 5e-4f * t;

		intersection.p = ray.point(t);
		intersection.t = transformation.rotation.x;
		intersection.b = transformation.rotation.y;
		intersection.n = normal;

		intersection.uv.x = math::dot(intersection.t, intersection.p);
		intersection.uv.y = math::dot(intersection.b, intersection.p);

		intersection.material_index = 0;

		hit_t = t;
		return true;
	}

	return false;
}

bool Plane::intersect_p(const Composed_transformation& transformation, const math::Oray& ray,
						const math::float2& /*bounds*/, Node_stack& /*node_stack*/) const {
	const math::float3& normal = transformation.rotation.z;
	float d = -math::dot(normal, transformation.position);
	float denom = math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) + d;
	float t = -(numer / denom);

	if (t > ray.min_t && t < ray.max_t) {
		return true;
	}

	return false;
}

float Plane::opacity(const Composed_transformation& transformation, const math::Oray& ray,
					 const math::float2& /*bounds*/, Node_stack& /*node_stack*/,
					 const material::Materials& materials, const image::sampler::Sampler_2D& sampler) const {
	const math::float3& normal = transformation.rotation.z;
	float d = -math::dot(normal, transformation.position);
	float denom = math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) + d;
	float t = -(numer / denom);

	if (t > ray.min_t && t < ray.max_t) {
		math::float3 p = ray.point(t);
		math::float2 uv(math::dot(transformation.rotation.x, p), math::dot(transformation.rotation.y, p));

		return materials[0]->opacity(uv, sampler);
	}

	return 0.f;
}

void Plane::importance_sample(uint32_t /*part*/, const Composed_transformation& /*transformation*/, const math::float3& /*p*/,
							  sampler::Sampler& /*sampler*/, uint32_t /*sample_index*/,
							  math::float3& /*wi*/, float& /*t*/, float& pdf) const {
	pdf = 0.f;
}

float Plane::area(uint32_t /*part*/, const math::float3& /*scale*/) const {
	return 1.f;
}

bool Plane::is_finite() const {
	return false;
}

}}
