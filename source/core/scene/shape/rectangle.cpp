#include "rectangle.hpp"
#include "shape_sample.hpp"
#include "shape_intersection.hpp"
#include "scene/scene_ray.inl"
#include "scene/scene_worker.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "sampler/sampler.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/sampling/sampling.inl"

namespace scene { namespace shape {

Rectangle::Rectangle() {
	aabb_.set_min_max(float3(-10.f, -10.f, -0.1f), float3(10.f, 10.f, 0.1f));
}

bool Rectangle::intersect(const Transformation& transformation, Ray& ray,
						  Node_stack& /*node_stack*/, Intersection& intersection) const {
	float3_p normal = transformation.rotation.v3.z;
	float d = math::dot(normal, transformation.position);
	float denom = -math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) - d;
	float hit_t = numer / denom;

	if (hit_t > ray.min_t && hit_t < ray.max_t) {
		float3 p = ray.point(hit_t);
		float3 k = p - transformation.position;

		float3 t = -transformation.rotation.v3.x;

		float u = math::dot(t, k / transformation.scale.x);
		if (u > 1.f || u < -1.f) {
			return false;
		}

		float3 b = -transformation.rotation.v3.y;

		float v = math::dot(b, k / transformation.scale.y);
		if (v > 1.f || v < -1.f) {
			return false;
		}

		intersection.epsilon = 5e-4f * hit_t;

		intersection.p = p;
		intersection.t = t;
		intersection.b = b;
		intersection.n = normal;
		intersection.geo_n = normal;
		intersection.uv.x = 0.5f * (u + 1.f);
		intersection.uv.y = 0.5f * (v + 1.f);

		intersection.part = 0;

		ray.max_t = hit_t;
		return true;
	}

	return false;
}

bool Rectangle::intersect_p(const Transformation& transformation,
							const Ray& ray, Node_stack& /*node_stack*/) const {
	float3_p normal = transformation.rotation.v3.z;
	float d = math::dot(normal, transformation.position);
	float denom = -math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) - d;
	float hit_t = numer / denom;

	if (hit_t > ray.min_t && hit_t < ray.max_t) {
		float3 p = ray.point(hit_t);
		float3 k = p - transformation.position;

		float3 t = -transformation.rotation.v3.x;

		float u = math::dot(t, k / transformation.scale.x);
		if (u > 1.f || u < -1.f) {
			return false;
		}

		float3 b = -transformation.rotation.v3.y;

		float v = math::dot(b, k / transformation.scale.y);
		if (v > 1.f || v < -1.f) {
			return false;
		}

		return true;
	}

	return false;
}

float Rectangle::opacity(const Transformation& transformation, const Ray& ray,
						 const material::Materials& materials,
						 Worker& worker, Sampler_filter filter) const {
	float3_p normal = transformation.rotation.v3.z;
	float d = math::dot(normal, transformation.position);
	float denom = -math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) - d;
	float hit_t = numer / denom;

	if (hit_t > ray.min_t && hit_t < ray.max_t) {
		float3 p = ray.point(hit_t);
		float3 k = p - transformation.position;

		float3 t = -transformation.rotation.v3.x;

		float u = math::dot(t, k / transformation.scale.x);
		if (u > 1.f || u < -1.f) {
			return 0.f;
		}

		float3 b = -transformation.rotation.v3.y;

		float v = math::dot(b, k / transformation.scale.y);
		if (v > 1.f || v < -1.f) {
			return 0.f;
		}

		float2 uv(0.5f * (u + 1.f), 0.5f * (v + 1.f));
		return materials[0]->opacity(uv, ray.time, worker, filter);
	}

	return 0.f;
}

float3 Rectangle::thin_absorption(const Transformation& transformation, const Ray& ray,
								  const material::Materials& materials,
								  Worker& worker, Sampler_filter filter) const {
	float3_p normal = transformation.rotation.v3.z;
	float d = math::dot(normal, transformation.position);
	float denom = -math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) - d;
	float hit_t = numer / denom;

	if (hit_t > ray.min_t && hit_t < ray.max_t) {
		float3 p = ray.point(hit_t);
		float3 k = p - transformation.position;

		float3 t = -transformation.rotation.v3.x;

		float u = math::dot(t, k / transformation.scale.x);
		if (u > 1.f || u < -1.f) {
			return float3(0.f);
		}

		float3 b = -transformation.rotation.v3.y;

		float v = math::dot(b, k / transformation.scale.y);
		if (v > 1.f || v < -1.f) {
			return float3(0.f);
		}

		float2 uv(0.5f * (u + 1.f), 0.5f * (v + 1.f));
		return materials[0]->thin_absorption(ray.direction, normal, uv,
											 ray.time, worker, filter);
	}

	return float3(0.f);
}

void Rectangle::sample(uint32_t part, const Transformation& transformation,
					   float3_p p, float3_p /*n*/, float area, bool two_sided,
					   sampler::Sampler& sampler, uint32_t sampler_dimension,
					   Node_stack& node_stack, Sample& sample) const {
	Rectangle::sample(part, transformation, p, area, two_sided,
					  sampler, sampler_dimension, node_stack, sample);
}

void Rectangle::sample(uint32_t /*part*/, const Transformation& transformation,
					   float3_p p, float area, bool two_sided,
					   sampler::Sampler& sampler, uint32_t sampler_dimension,
					   Node_stack& /*node_stack*/, Sample& sample) const {
	float2 r2 = sampler.generate_sample_2D(sampler_dimension);
	float2 xy = float2(2.f * r2 - 1.f);

	float3 scale(transformation.scale.xy, 1.f);

	float3 ls = float3(xy, 0.f);
	float3 ws = transformation.position
			  + math::transform_vector(scale * ls, transformation.rotation);

	float3 axis = ws - p;

	float sl = math::squared_length(axis);
	float t  = std::sqrt(sl);

	float3 wi = axis / t;

	float c = -math::dot(transformation.rotation.v3.z, wi);

	if (two_sided) {
		c = std::abs(c);
	}

	if (c <= 0.f) {
		sample.pdf = 0.f;
	} else {
		sample.wi = wi;
		sample.t = t;
		sample.pdf = sl / (c * area);
	}
}

float Rectangle::pdf(uint32_t /*part*/, const Transformation& transformation,
					 float3_p p, float3_p wi, float area, bool two_sided,
					 bool /*total_sphere*/, Node_stack& /*node_stack*/) const {
	float3 normal = transformation.rotation.v3.z;

	float denom = -math::dot(normal, wi);
	float c = denom;

	if (two_sided) {
		c = std::abs(c);
	}

	if (c <= 0.f) {
		return 0.f;
	}

	float d = math::dot(normal, transformation.position);
	float numer = math::dot(normal, p) - d;
	float hit_t = numer / denom;

	float3 ws = p + hit_t * wi; // ray.point(t);
	float3 k = ws - transformation.position;

	float3 t = -transformation.rotation.v3.x;

	float u = math::dot(t, k / transformation.scale.x);
	if (u > 1.f || u < -1.f) {
		return 0.f;
	}

	float3 b = -transformation.rotation.v3.y;

	float v = math::dot(b, k / transformation.scale.y);
	if (v > 1.f || v < -1.f) {
		return 0.f;
	}

	float sl = hit_t * hit_t;
	return sl / (c * area);
}

void Rectangle::sample(uint32_t /*part*/, const Transformation& transformation,
					   float3_p p, float2 uv, float area, bool two_sided,
					   Sample& sample) const {
	float3 ls(-2.f * uv.x + 1.f, -2.f * uv.y + 1.f, 0.f);
	float3 ws = math::transform_point(ls, transformation.object_to_world);

	float3 axis = ws - p;
	float sl = math::squared_length(axis);
	float d = std::sqrt(sl);

	float3 dir = axis / d;

	float3 wn = transformation.rotation.v3.z;

	float c = -math::dot(wn, dir);

	if (two_sided) {
		c = std::abs(c);
	}

	if (c <= 0.f) {
		sample.pdf = 0.f;
	} else {
		sample.wi = dir;
		sample.uv = uv;
		sample.t  = d;
		// sin_theta because of the uv weight
		sample.pdf = sl / (c * area /** sin_theta*/);
	}
}

float Rectangle::pdf_uv(uint32_t /*part*/, const Transformation& transformation,
						float3_p p, float3_p wi, float area, bool two_sided,
						float2& uv) const {
	float3 normal = transformation.rotation.v3.z;

	float denom = -math::dot(normal, wi);
	float c = denom;

	if (two_sided) {
		c = std::abs(c);
	}

	if (c <= 0.f) {
		return 0.f;
	}

	float d = math::dot(normal, transformation.position);
	float numer = math::dot(normal, p) - d;
	float hit_t = numer / denom;

	float3 ws = p + hit_t * wi; // ray.point(t);
	float3 k = ws - transformation.position;

	float3 t = -transformation.rotation.v3.x;

	float u = math::dot(t, k / transformation.scale.x);
	if (u > 1.f || u < -1.f) {
		return 0.f;
	}

	float3 b = -transformation.rotation.v3.y;

	float v = math::dot(b, k / transformation.scale.y);
	if (v > 1.f || v < -1.f) {
		return 0.f;
	}

	uv = float2(0.5f * (u + 1.f), 0.5f * (v + 1.f));

	float sl = hit_t * hit_t;
	return sl / (c * area);
}

float Rectangle::uv_weight(float2 /*uv*/) const {
	return 1.f;
}

float Rectangle::area(uint32_t /*part*/, float3_p scale) const {
	return 4.f * scale.x * scale.y;
}

size_t Rectangle::num_bytes() const {
	return sizeof(*this);
}

}}

