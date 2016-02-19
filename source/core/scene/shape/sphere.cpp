#include "sphere.hpp"
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

Sphere::Sphere() {
	aabb_.set_min_max(math::float3(-1.f, -1.f, -1.f), math::float3(1.f, 1.f, 1.f));
}

bool Sphere::intersect(const entity::Composed_transformation& transformation, math::Oray& ray,
					   Node_stack& /*node_stack*/, Intersection& intersection) const {
	math::float3 v = transformation.position - ray.origin;
	float b = math::dot(v, ray.direction);
	float radius = transformation.scale.x;
	float det = (b * b) - math::dot(v, v) + (radius * radius);

	if (det > 0.f) {
		float dist = std::sqrt(det);
		float t0 = b - dist;

		if (t0 > ray.min_t && t0 < ray.max_t) {
			intersection.epsilon = 5e-4f * t0;

			intersection.p = ray.point(t0);
			intersection.n = math::normalized(intersection.p - transformation.position);
			math::coordinate_system(intersection.n, intersection.t, intersection.b);
			intersection.geo_n = intersection.n;

			math::float3 xyz = math::normalized(
						math::transform_vector_transposed(intersection.n, transformation.rotation));
			intersection.uv = math::float2(-std::atan2(xyz.x, xyz.z) * math::Pi_inv * 0.5f + 0.5f,
										   std::acos(xyz.y) * math::Pi_inv);

			math::float3 n = xyz;//intersection.n;
			math::float2 uv = math::float2(-std::atan2(n.x, n.z) + math::Pi, std::acos(n.y));

			float phi   = uv.x;
			float theta = uv.y;

			float sin_theta = std::sin(theta);
			float sin_phi   = std::sin(phi);
			float cos_phi   = std::cos(phi);

			math::float3 t(sin_theta * cos_phi, 0.f, sin_theta * sin_phi);

			t = math::normalized(math::transform_vector(t, transformation.rotation));

			intersection.t = t;
			intersection.b = -math::cross(t, n);

			intersection.part = 0;

			ray.max_t = t0;
			return true;
		}

		float t1 = b + dist;

		if (t1 > ray.min_t && t1 < ray.max_t) {
			intersection.epsilon = 5e-4f * t1;

			intersection.p = ray.point(t1);
			intersection.n = math::normalized(intersection.p - transformation.position);
			math::coordinate_system(intersection.n, intersection.t, intersection.b);
			intersection.geo_n = intersection.n;

			math::float3 xyz = math::normalized(
						math::transform_vector_transposed(intersection.n, transformation.rotation));
			intersection.uv = math::float2(-std::atan2(xyz.x, xyz.z) * math::Pi_inv * 0.5f + 0.5f,
										   std::acos(xyz.y) * math::Pi_inv);

			intersection.part = 0;

			ray.max_t = t1;
			return true;
		}
	}

	return false;
}

bool Sphere::intersect_p(const entity::Composed_transformation& transformation, const math::Oray& ray,
						 Node_stack& /*node_stack*/) const {
	math::float3 v = transformation.position - ray.origin;
	float b = math::dot(v, ray.direction);
	float radius = transformation.scale.x;
	float det = (b * b) - math::dot(v, v) + (radius * radius);

	if (det > 0.f) {
		float dist = std::sqrt(det);
		float t0 = b - dist;

		if (t0 > ray.min_t && t0 < ray.max_t) {
			return true;
		}

		float t1 = b + dist;

		if (t1 > ray.min_t && t1 < ray.max_t) {
			return true;
		}
	}

	return false;
}

float Sphere::opacity(const entity::Composed_transformation& transformation, const math::Oray& ray,
					  float time, Node_stack& /*node_stack*/, const material::Materials& materials,
					  const image::texture::sampler::Sampler_2D& sampler) const {
	math::float3 v = transformation.position - ray.origin;
	float b = dot(v, ray.direction);
	float radius = transformation.scale.x;
	float det = (b * b) - dot(v, v) + (radius * radius);

	if (det > 0.f) {
		float dist = std::sqrt(det);
		float t0 = b - dist;

		if (t0 > ray.min_t && t0 < ray.max_t) {
			math::float3 n = math::normalized(ray.point(t0) - transformation.position);
			math::float3 xyz = math::normalized(
						math::transform_vector_transposed(n, transformation.rotation));
			math::float2 uv = math::float2(-std::atan2(xyz.x, xyz.z) * math::Pi_inv * 0.5f + 0.5f,
										   std::acos(xyz.y) * math::Pi_inv);

			return materials[0]->opacity(uv, time, sampler);
		}

		float t1 = b + dist;

		if (t1 > ray.min_t && t1 < ray.max_t) {
			math::float3 n = math::normalized(ray.point(t1) - transformation.position);
			math::float3 xyz = math::normalized(
						math::transform_vector_transposed(n, transformation.rotation));
			math::float2 uv = math::float2(-std::atan2(xyz.x, xyz.z) * math::Pi_inv * 0.5f + 0.5f,
										   std::acos(xyz.y) * math::Pi_inv);

			return materials[0]->opacity(uv, time, sampler);
		}
	}

	return 0.f;
}

void Sphere::sample(uint32_t part, const entity::Composed_transformation& transformation, float area,
					const math::float3& p, const math::float3& /*n*/, bool two_sided,
					sampler::Sampler& sampler, Node_stack& node_stack, Sample& sample) const {
	Sphere::sample(part, transformation, area, p, two_sided, sampler, node_stack, sample);
}

void Sphere::sample(uint32_t /*part*/, const entity::Composed_transformation& transformation, float /*area*/,
					const math::float3& p, bool /*two_sided*/,
					sampler::Sampler& sampler, Node_stack& /*node_stack*/, Sample& sample) const {
	math::float3 axis = transformation.position - p;
	float axis_squared_length = math::squared_length(axis);

	float radius_square = transformation.scale.x * transformation.scale.x;
	float sin_theta_max2 = radius_square / axis_squared_length;
	float cos_theta_max  = std::sqrt(std::max(0.f, 1.f - sin_theta_max2));
	cos_theta_max = std::min(0.99999995f, cos_theta_max);

	float axis_length = std::sqrt(axis_squared_length);
	math::float3 z = axis / axis_length;
	math::float3 x, y;
	math::coordinate_system(z, x, y);

	math::float2 r2 = sampler.generate_sample_2D();
	math::float3 dir = math::sample_oriented_cone_uniform(r2, cos_theta_max, x, y, z);

	sample.wi = dir;
	sample.t = axis_length - transformation.scale.x; // this is not accurate
	sample.pdf = math::cone_pdf_uniform(cos_theta_max);

//	if (std::isinf(sample.pdf)) {
//		sample.pdf = 1.f;
//	}
}

void Sphere::sample(uint32_t /*part*/, const entity::Composed_transformation& transformation, float area,
					const math::float3& p, math::float2 uv, Sample& sample) const {
	float phi   = (uv.x + 0.75f) * 2.f * math::Pi;
	float theta = uv.y * math::Pi;

	float sin_theta = std::sin(theta);
	float cos_theta = std::cos(theta);
	float sin_phi   = std::sin(phi);
	float cos_phi   = std::cos(phi);

	math::float3 ls(sin_theta * cos_phi, cos_theta, sin_theta * sin_phi);
	math::float3 ws = math::transform_point(ls, transformation.object_to_world);

	math::float3 axis = ws - p;
	float sl = math::squared_length(axis);
	float d = std::sqrt(sl);

	math::float3 dir = axis / d;

	math::float3 wn = math::normalized(ws - transformation.position);

	float c = math::dot(wn, -dir);

	if (c <= 0.f) {
		sample.pdf = 0.f;
	} else {
		sample.wi = dir;
		sample.uv = uv;
		sample.t  = d;
		sample.pdf = sl / (c * area);
	}
}

void Sphere::sample(uint32_t /*part*/, const entity::Composed_transformation& transformation, float area,
					const math::float3& p, const math::float3& wi, Sample& sample) const {
	math::float3 v = transformation.position - p;
	float b = math::dot(v, wi);
	float radius = transformation.scale.x;
	float det = (b * b) - math::dot(v, v) + (radius * radius);

	if (det > 0.f) {
		float dist = std::sqrt(det);

		float t = b - dist;
		math::float3 hit = p + t * wi;
		math::float3 wn = math::normalized(hit - transformation.position);

		math::float3 xyz = math::transform_vector_transposed(wn, transformation.rotation);
		sample.uv = math::float2(-std::atan2(xyz.x, xyz.z) * math::Pi_inv * 0.5f + 0.5f,
								 std::acos(xyz.y) * math::Pi_inv);

		float sl = t * t;
		float c = math::dot(wn, -wi);
		sample.pdf = sl / (c * area);
	} else {
		sample.pdf = 0.f;
	}
}

float Sphere::pdf(uint32_t /*part*/, const entity::Composed_transformation& transformation, float /*area*/,
				  const math::float3& p, const math::float3& wi, bool /*two_sided*/, bool /*total_sphere*/,
				  Node_stack& /*node_stack*/) const {
	math::float3 axis = transformation.position - p;
	float axis_squared_length = math::squared_length(axis);
	float radius_square = transformation.scale.x * transformation.scale.x;

	float b = math::dot(axis, wi);
	float det = (b * b) - axis_squared_length + radius_square;

	if (det <= 0.f) {
		return 0.f;
	}

	float sin_theta_max2 = radius_square / axis_squared_length;
	float cos_theta_max  = std::sqrt(std::max(0.f, 1.f - sin_theta_max2));
	cos_theta_max = std::min(0.99999995f, cos_theta_max);

	return math::cone_pdf_uniform(cos_theta_max);
}

float Sphere::area(uint32_t /*part*/, const math::float3& scale) const {
	return 4.f * math::Pi * scale.x * scale.x;
}

}}
