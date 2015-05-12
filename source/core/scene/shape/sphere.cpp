#include "sphere.hpp"
#include "geometry/shape_intersection.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "sampler/sampler.hpp"
#include "base/math/sampling.hpp"
#include "base/math/vector.inl"
#include "base/math/ray.inl"
#include "base/math/bounding/aabb.inl"

namespace scene { namespace shape {

Sphere::Sphere() {
	aabb_.set_min_max(math::float3(-1.f, -1.f, -1.f), math::float3(1.f, 1.f, 1.f));
}

bool Sphere::intersect(const Composed_transformation& transformation, const math::Oray& ray,
					   const math::float2& /*bounds*/, Node_stack& /*node_stack*/,
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

bool Sphere::intersect_p(const Composed_transformation& transformation, const math::Oray& ray,
						 const math::float2& /*bounds*/, Node_stack& /*node_stack*/) const {
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

float Sphere::opacity(const Composed_transformation& transformation, const math::Oray& ray,
					  const math::float2& bounds, Node_stack& node_stack,
					  const material::Materials& /*materials*/, const image::sampler::Sampler_2D& /*sampler*/) const {
	return intersect_p(transformation, ray, bounds, node_stack) ? 1.f : 0.f;
}

void Sphere::importance_sample(uint32_t /*part*/, const Composed_transformation& transformation, float /*area*/, const math::float3& p,
							   sampler::Sampler& sampler, uint32_t sample_index,
							   math::float3& wi, float& t, float& pdf) const {
	math::float3 axis = transformation.position - p;
	float axis_squared_length = math::squared_length(axis);
	float axis_length = std::sqrt(axis_squared_length);

	math::float3 z = axis / axis_length;
	math::float3 x, y;
	math::coordinate_system(z, x, y);

	float radius_square = transformation.scale.x * transformation.scale.x;
	float sin_theta_max2 = radius_square / axis_squared_length;
	float cos_theta_max  = std::sqrt(std::max(0.f, 1.f - sin_theta_max2));

	math::float2 sample = sampler.generate_sample2d(sample_index);
	math::float3 dir = math::sample_oriented_cone_uniform(sample, cos_theta_max, x, y, z);

	wi = dir;
	t = axis_length - transformation.scale.x; // this is not accurate
	pdf = math::cone_pdf_uniform(cos_theta_max);
}

float Sphere::area(uint32_t /*part*/, const math::float3& scale) const {
	return 4.f * math::Pi * scale.x * scale.x;
}

}}
