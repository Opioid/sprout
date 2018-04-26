#include "rectangle.hpp"
#include "shape_sample.hpp"
#include "shape_intersection.hpp"
#include "scene/scene_ray.inl"
#include "scene/scene_worker.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "sampler/sampler.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/sampling/sampling.hpp"

namespace scene::shape {

Rectangle::Rectangle() {
	aabb_.set_min_max(float3(-10.f, -10.f, -0.1f), float3(10.f, 10.f, 0.1f));
}

bool Rectangle::intersect(Ray& ray, const Transformation& transformation,
						  Node_stack& /*node_stack*/, Intersection& intersection) const {
	const float3& normal = transformation.rotation.r[2];
	float d = math::dot(normal, transformation.position);
	float denom = -math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) - d;
	float hit_t = numer / denom;

	if (hit_t > ray.min_t && hit_t < ray.max_t) {
		float3 p = ray.point(hit_t);
		float3 k = p - transformation.position;

		float3 t = -transformation.rotation.r[0];

		float u = math::dot(t, k / transformation.scale[0]);
		if (u > 1.f || u < -1.f) {
			return false;
		}

		float3 b = -transformation.rotation.r[1];

		float v = math::dot(b, k / transformation.scale[1]);
		if (v > 1.f || v < -1.f) {
			return false;
		}

		intersection.epsilon = 5e-4f * hit_t;

		intersection.p = p;
		intersection.t = t;
		intersection.b = b;
		intersection.n = normal;
		intersection.geo_n = normal;
		intersection.uv[0] = 0.5f * (u + 1.f);
		intersection.uv[1] = 0.5f * (v + 1.f);

		intersection.part = 0;

		ray.max_t = hit_t;
		return true;
	}

	return false;
}

bool Rectangle::intersect_fast(Ray& ray, const Transformation& transformation,
							   Node_stack& /*node_stack*/, Intersection& intersection) const {
	const float3& normal = transformation.rotation.r[2];
	float d = math::dot(normal, transformation.position);
	float denom = -math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) - d;
	float hit_t = numer / denom;

	if (hit_t > ray.min_t && hit_t < ray.max_t) {
		float3 p = ray.point(hit_t);
		float3 k = p - transformation.position;

		float3 t = -transformation.rotation.r[0];

		float u = math::dot(t, k / transformation.scale[0]);
		if (u > 1.f || u < -1.f) {
			return false;
		}

		float3 b = -transformation.rotation.r[1];

		float v = math::dot(b, k / transformation.scale[1]);
		if (v > 1.f || v < -1.f) {
			return false;
		}

		intersection.epsilon = 5e-4f * hit_t;

		intersection.p = p;
		intersection.geo_n = normal;
		intersection.uv[0] = 0.5f * (u + 1.f);
		intersection.uv[1] = 0.5f * (v + 1.f);

		intersection.part = 0;

		ray.max_t = hit_t;
		return true;
	}

	return false;
}

bool Rectangle::intersect(Ray& ray, const Transformation& transformation,
						  Node_stack& /*node_stack*/, float& epsilon) const {
	const float3& normal = transformation.rotation.r[2];
	float d = math::dot(normal, transformation.position);
	float denom = -math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) - d;
	float hit_t = numer / denom;

	if (hit_t > ray.min_t && hit_t < ray.max_t) {
		float3 p = ray.point(hit_t);
		float3 k = p - transformation.position;

		float3 t = -transformation.rotation.r[0];

		float u = math::dot(t, k / transformation.scale[0]);
		if (u > 1.f || u < -1.f) {
			return false;
		}

		float3 b = -transformation.rotation.r[1];

		float v = math::dot(b, k / transformation.scale[1]);
		if (v > 1.f || v < -1.f) {
			return false;
		}

		ray.max_t = hit_t;
		epsilon = 5e-4f * hit_t;
		return true;
	}

	return false;
}

bool Rectangle::intersect_p(const Ray& ray, const Transformation& transformation,
							Node_stack& /*node_stack*/) const {
	const float3& normal = transformation.rotation.r[2];
	float d = math::dot(normal, transformation.position);
	float denom = -math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) - d;
	float hit_t = numer / denom;

	if (hit_t > ray.min_t && hit_t < ray.max_t) {
		float3 p = ray.point(hit_t);
		float3 k = p - transformation.position;

		float3 t = -transformation.rotation.r[0];

		float u = math::dot(t, k / transformation.scale[0]);
		if (u > 1.f || u < -1.f) {
			return false;
		}

		float3 b = -transformation.rotation.r[1];

		float v = math::dot(b, k / transformation.scale[1]);
		if (v > 1.f || v < -1.f) {
			return false;
		}

		return true;
	}

	return false;
}

float Rectangle::opacity(const Ray& ray, const Transformation& transformation,
						 const Materials& materials,
						 Sampler_filter filter, const Worker& worker) const {
	const float3& normal = transformation.rotation.r[2];
	float d = math::dot(normal, transformation.position);
	float denom = -math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) - d;
	float hit_t = numer / denom;

	if (hit_t > ray.min_t && hit_t < ray.max_t) {
		float3 p = ray.point(hit_t);
		float3 k = p - transformation.position;

		float3 t = -transformation.rotation.r[0];

		float u = math::dot(t, k / transformation.scale[0]);
		if (u > 1.f || u < -1.f) {
			return 0.f;
		}

		float3 b = -transformation.rotation.r[1];

		float v = math::dot(b, k / transformation.scale[1]);
		if (v > 1.f || v < -1.f) {
			return 0.f;
		}

		float2 uv(0.5f * (u + 1.f), 0.5f * (v + 1.f));
		return materials[0]->opacity(uv, ray.time, filter, worker);
	}

	return 0.f;
}

float3 Rectangle::thin_absorption(const Ray& ray, const Transformation& transformation,
								  const Materials& materials,
								  Sampler_filter filter, const Worker& worker) const {
	const float3& normal = transformation.rotation.r[2];
	float d = math::dot(normal, transformation.position);
	float denom = -math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) - d;
	float hit_t = numer / denom;

	if (hit_t > ray.min_t && hit_t < ray.max_t) {
		float3 p = ray.point(hit_t);
		float3 k = p - transformation.position;

		float3 t = -transformation.rotation.r[0];

		float u = math::dot(t, k / transformation.scale[0]);
		if (u > 1.f || u < -1.f) {
			return float3(0.f);
		}

		float3 b = -transformation.rotation.r[1];

		float v = math::dot(b, k / transformation.scale[1]);
		if (v > 1.f || v < -1.f) {
			return float3(0.f);
		}

		float2 uv(0.5f * (u + 1.f), 0.5f * (v + 1.f));
		return materials[0]->thin_absorption(ray.direction, normal, uv,
											 ray.time, filter, worker);
	}

	return float3(0.f);
}

bool Rectangle::sample(uint32_t part, f_float3 p, f_float3 /*n*/,
					   const Transformation& transformation, float area, bool two_sided,
					   sampler::Sampler& sampler, uint32_t sampler_dimension,
					   Node_stack& node_stack, Sample& sample) const {
	return Rectangle::sample(part, p, transformation, area, two_sided,
							 sampler, sampler_dimension, node_stack, sample);
}

bool Rectangle::sample(uint32_t /*part*/, f_float3 p, const Transformation& transformation,
					   float area, bool two_sided,
					   sampler::Sampler& sampler, uint32_t sampler_dimension,
					   Node_stack& /*node_stack*/, Sample& sample) const {
	float2 r2 = sampler.generate_sample_2D(sampler_dimension);
	float2 xy = 2.f * r2 - float2(1.f);

	float3 scale(transformation.scale.xy(), 1.f);

	float3 ls = float3(xy, 0.f);
	float3 ws = transformation.position
			  + math::transform_vector(scale * ls, transformation.rotation);

	float3 axis = ws - p;

	float sl = math::squared_length(axis);
	float t  = std::sqrt(sl);

	float3 wi = axis / t;

	float c = -math::dot(transformation.rotation.r[2], wi);

	if (two_sided) {
		c = std::abs(c);
	}

	if (c <= 0.f) {
		return false;
	}

	sample.wi = wi;
	sample.pdf = sl / (c * area);
	sample.t = t;
	sample.epsilon = 5e-4f * t;

	return true;
}

float Rectangle::pdf(const Ray& ray, const shape::Intersection& /*intersection*/,
					 const Transformation& transformation,
					 float area, bool two_sided, bool /*total_sphere*/) const {
	const float3 normal = transformation.rotation.r[2];

	float c = -math::dot(normal, ray.direction);

	if (two_sided) {
		c = std::abs(c);
	}

	const float sl = ray.max_t * ray.max_t;
	return sl / (c * area);
}

bool Rectangle::sample(uint32_t /*part*/, f_float3 p, float2 uv,
					   const Transformation& transformation, float area, bool two_sided,
					   Sample& sample) const {
	float3 ls(-2.f * uv[0] + 1.f, -2.f * uv[1] + 1.f, 0.f);
	float3 ws = math::transform_point(ls, transformation.object_to_world);

	float3 axis = ws - p;
	float sl = math::squared_length(axis);
	float d = std::sqrt(sl);

	float3 dir = axis / d;

	float3 wn = transformation.rotation.r[2];

	float c = -math::dot(wn, dir);

	if (two_sided) {
		c = std::abs(c);
	}

	if (c <= 0.f) {
		return false;
	}

	sample.wi = dir;
	sample.uv = uv;
	sample.t  = d;
	// sin_theta because of the uv weight
	sample.pdf = sl / (c * area /** sin_theta*/);

	return true;
}

float Rectangle::pdf_uv(const Ray& ray, const Intersection& intersection,
						const Transformation& transformation,
						float area, bool two_sided) const {
	return pdf(ray, intersection, transformation, area, two_sided, false);
}

float Rectangle::uv_weight(float2 /*uv*/) const {
	return 1.f;
}

float Rectangle::area(uint32_t /*part*/, f_float3 scale) const {
	return 4.f * scale[0] * scale[1];
}

size_t Rectangle::num_bytes() const {
	return sizeof(*this);
}

}
