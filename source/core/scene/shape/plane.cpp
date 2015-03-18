#include "plane.hpp"
#include "geometry/shape_intersection.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "base/math/ray.inl"

namespace scene { namespace shape {

Plane::Plane() {
	aabb_.set_min_max(math::float3::identity, math::float3::identity);
}

bool Plane::intersect(const Composed_transformation& transformation, const math::Oray& ray, const math::float2& bounds,
					  Intersection& intersection, float& hit_t) const {
	/*
normal := transformation.Rotation.Direction()

d := -normal.Dot(transformation.Position)

denom := normal.Dot(ray.Direction)

numer := normal.Dot(ray.Origin) + d

thit := -(numer / denom)

if thit > ray.MinT && thit < ray.MaxT {
	intersection.Epsilon = 5e-4 * thit

	intersection.P = ray.Point(thit)
	intersection.T = transformation.Rotation.Right()
	intersection.B = transformation.Rotation.Up()
	intersection.N = normal

	u := transformation.ObjectToWorld.Right().Dot(intersection.P)
	intersection.UV.X = u - math32.Floor(u)

	v := transformation.ObjectToWorld.Up().Dot(intersection.P)
	intersection.UV.Y = v - math32.Floor(v)

	intersection.MaterialIndex = 0

	return true, thit
} 	*/

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

		intersection.material_index = 0;

		hit_t = t;
		return true;
	}

	return false;
}

bool Plane::intersect_p(const Composed_transformation& transformation, const math::Oray& ray, const math::float2& bounds) const {
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

bool Plane::is_finite() const {
	return false;
}

}}
