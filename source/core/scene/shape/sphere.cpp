#include "sphere.hpp"
#include "geometry/shape_intersection.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "base/math/vector.inl"
#include "base/math/ray.inl"

namespace scene { namespace shape {

Sphere::Sphere() {
	aabb_.set_min_max(math::float3(-1.f, -1.f, -1.f), math::float3(1.f, 1.f, 1.f));
}

bool Sphere::intersect(const Composed_transformation& transformation, const math::Oray& ray, const math::float2& bounds,
					   Intersection& intersection, float& hit_t) const {
	/*
v := ray.Origin.Sub(transformation.Position)
b := -v.Dot(ray.Direction)
radius := transformation.Scale.X
det := (b * b) - v.Dot(v) + (radius * radius)

if det > 0.0 {

	dist := math32.Sqrt(det)
	t0 := b - dist

	if t0 > ray.MinT && t0 < ray.MaxT {
		intersection.Epsilon = 5e-4 * t0

		intersection.P = ray.Point(t0)
		intersection.N = intersection.P.Sub(transformation.Position).Div(radius)

		t, b := math.CoordinateSystem(intersection.N)
		intersection.T = t
		intersection.B = b

		intersection.MaterialIndex = 0

		return true, t0
	}

	t1 := b + dist

	if t1 > ray.MinT && t1 < ray.MaxT {
		intersection.Epsilon = 5e-4 * t1

		intersection.P = ray.Point(t1)
		intersection.N = intersection.P.Sub(transformation.Position).Div(radius)

		t, b := math.CoordinateSystem(intersection.N)
		intersection.T = t
		intersection.B = b

		intersection.MaterialIndex = 0

		return true, t1
	}

}

return false, 0.0
	*/

	math::float3 v = ray.origin - transformation.position;
	float b = -dot(v, ray.direction);
	float radius = transformation.scale.x;
	float det = (b * b) - dot(v, v) + (radius * radius);

	if (det > 0.f) {
		float dist = std::sqrt(det);
		float t0 = b - dist;

		if (t0 > ray.min_t && t0 < ray.max_t) {
			intersection.epsilon = 5e-4f * t0;

			intersection.p = ray.point(t0);
			intersection.n = (intersection.p - transformation.position) / radius;
			math::coordinate_system(intersection.n, intersection.t, intersection.b);

			intersection.material_index = 0;

			hit_t = t0;
			return true;
		}

		float t1 = b + dist;

		if (t1 > ray.min_t && t1 < ray.max_t) {
			intersection.epsilon = 5e-4f * t1;

			intersection.p = ray.point(t1);
			intersection.n = (intersection.p - transformation.position) / radius;
			math::coordinate_system(intersection.n, intersection.t, intersection.b);

			intersection.material_index = 0;

			hit_t = t1;
			return true;
		}
	}

	return false;
}

bool Sphere::intersect_p(const Composed_transformation& transformation, const math::Oray& ray, const math::float2& bounds) const {
	math::float3 v = ray.origin - transformation.position;
	float b = -dot(v, ray.direction);
	float radius = transformation.scale.x;
	float det = (b * b) - dot(v, v) + (radius * radius);

	if (det > 0.f) {
		float dist = std::sqrt(det);
		float t0 = b - dist;
		float t1 = b + dist;

		if (t1 > ray.min_t && t0 < ray.max_t) {
			return true;
		}

		if (t0 > ray.min_t && t1 < ray.max_t) {
			return true;
		}
	}

	return false;
}

}}
