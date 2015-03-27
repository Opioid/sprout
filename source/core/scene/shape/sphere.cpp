#include "sphere.hpp"
#include "geometry/shape_intersection.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "base/math/vector.inl"
#include "base/math/ray.inl"

namespace scene { namespace shape {

Sphere::Sphere() {
	aabb_.set_min_max(math::float3(-1.f, -1.f, -1.f), math::float3(1.f, 1.f, 1.f));
}

bool Sphere::intersect(const Composed_transformation& transformation, const math::Oray& ray, const math::float2& /*bounds*/,
					   Intersection& intersection, float& hit_t) const {
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

bool Sphere::intersect_p(const Composed_transformation& transformation, const math::Oray& ray, const math::float2& /*bounds*/) const {
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

void Sphere::importance_sample(const Composed_transformation& transformation, const math::float3& p, const math::float2& sample,
							   math::float3& wi, float& t, float& pdf) const {
/*
	l.prop.TransformationAt(time, transformation)

	axis := transformation.Position.Sub(p)
	axisSquaredLength := axis.SquaredLength()
	axisrl := math32.Rsqrt(axisSquaredLength)

	z := axis.Scale(axisrl)
	x, y := math.CoordinateSystem(z)

	radiusSquare := transformation.Scale.X * transformation.Scale.X

	sinThetaMax2 := radiusSquare / axisSquaredLength
	cosThetaMax := math32.Sqrt(math32.Max(0.0, 1.0 - sinThetaMax2))

	sample := sampler.GenerateSample2D(0, subsample)
	dir := math.SampleOrientedConeUniform(sample.X, sample.Y, cosThetaMax, x, y, z)

	t := dir.Dot(dir) * axis.Dot(dir)

	w := dir

	result := Sample{Energy: l.color.Scale(l.lumen), L: w, T: t, Pdf: math.ConePdfUniform(cosThetaMax)}
	*/


}

}}
