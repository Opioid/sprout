#include "rectangle.hpp"
#include "shape_sample.hpp"
#include "shape_intersection.hpp"
#include "scene/scene_ray.inl"
#include "scene/scene_worker.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "sampler/sampler.hpp"
#include "base/math/sampling/sampling.inl"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/bounding/aabb.inl"

namespace scene { namespace shape {

Rectangle::Rectangle() {
	aabb_.set_min_max(float3(-1.f, -1.f, -0.1f), float3(1.f, 1.f, 0.1f));
}

bool Rectangle::intersect(const Transformation& transformation, Ray& ray,
						  Node_stack& /*node_stack*/, Intersection& intersection) const {
	/*
	const float3& normal = transformation.rotation.v3.z;
	float d = math::dot(normal, transformation.position);
	float denom = math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) - d;
	float t = -(numer / denom);

	if (t > ray.min_t && t < ray.max_t) {
		float3 p = ray.point(t);
		float3 k = p - transformation.position;
		float l = math::dot(k, k);

		float radius = transformation.scale.x;

		if (l <= radius * radius) {
			intersection.epsilon = 5e-4f * t;

			intersection.p = p;
			intersection.t = -transformation.rotation.v3.x;
			intersection.b = -transformation.rotation.v3.y;
			intersection.n = normal;
			intersection.geo_n = normal;
			float3 sk = k / radius;

			intersection.uv.x = (math::dot(intersection.t, sk) + 1.f)
								 * 0.5f * transformation.scale.z;
			intersection.uv.y = (math::dot(intersection.b, sk) + 1.f)
								 * 0.5f * transformation.scale.z;

			intersection.part = 0;

			ray.max_t = t;
			return true;
		}
	}

	return false;
	*/
	/*
	Let V3 = P4 - P3 be the vector opposite from V1.  Let V4 be the vector
	from P1 to the point of intersection, and let V5 be the vector from P3
	to the point of intersection.  Normalize vectors V1, V3, V4 and V5.
	Then find the dot products V1 dot V4 and V3 dot V5. If both are
	non-negative, then the point is in the rectangle.  This works because
	we use the fact that if the angle between V1 and V4 and the angle
	between V3 and V5 are between 0 and 90 inclusive, the point is in the
	rectangle.
	*/

	float2 scale(transformation.scale.xy);

	float3 a = transformation.position
			  + math::transform_vector(float3(-scale.x, scale.y, 0.f), transformation.rotation);

	float3 b = transformation.position
			  + math::transform_vector(float3(scale.x, scale.y, 0.f), transformation.rotation);

	float3 c = transformation.position
			  + math::transform_vector(float3(-scale.x, -scale.y, 0.f), transformation.rotation);

	float3 e1 = b - a;
	float3 e2 = c - a;

	float3 pvec = math::cross(ray.direction, e2);

	float det = math::dot(e1, pvec);
	float inv_det = 1.f / det;

	float3 tvec = ray.origin - a;
	float u = math::dot(tvec, pvec) * inv_det;

	if (u < 0.f || u > 1.f) {
		return false;
	}

	float3 qvec = math::cross(tvec, e1);
	float v = math::dot(ray.direction, qvec) * inv_det;

	if (v < 0.f || u + v > 1.f) {
		return false;
	}

	float hit_t = math::dot(e2, qvec) * inv_det;

	if (hit_t > ray.min_t && hit_t < ray.max_t) {
		ray.max_t = hit_t;
	//	uv.x = u;
	//	uv.y = v;

		intersection.epsilon = 5e-4f * hit_t;
		intersection.p = ray.point(hit_t);
		intersection.t = -transformation.rotation.v3.x;
		intersection.b = -transformation.rotation.v3.y;
		intersection.n = transformation.rotation.v3.z;
		intersection.geo_n = transformation.rotation.v3.z;

		intersection.part = 0;

		return true;
	}

	return false;
}

bool Rectangle::intersect_p(const Transformation& transformation,
							const Ray& ray, Node_stack& /*node_stack*/) const {
	float3_p normal = transformation.rotation.v3.z;
	float d = math::dot(normal, transformation.position);
	float denom = math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) - d;
	float t = -(numer / denom);

	if (t > ray.min_t && t < ray.max_t) {
		float3 p = ray.point(t);
		float3 k = p - transformation.position;
		float l = math::dot(k, k);

		float radius = transformation.scale.x;

		if (l <= radius * radius) {
			return true;
		}
	}

	return false;
}

float Rectangle::opacity(const Transformation& transformation, const Ray& ray,
						 const material::Materials& materials,
						 Worker& worker, Sampler_filter filter) const {
	float3_p normal = transformation.rotation.v3.z;
	float d = math::dot(normal, transformation.position);
	float denom = math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) - d;
	float t = -(numer / denom);

	if (t > ray.min_t && t < ray.max_t) {
		float3 p = ray.point(t);
		float3 k = p - transformation.position;
		float l = math::dot(k, k);

		float radius = transformation.scale.x;

		if (l <= radius * radius) {
			float3 sk = k / radius;
			float2 uv((math::dot(transformation.rotation.v3.x, sk) + 1.f) * 0.5f,
					  (math::dot(transformation.rotation.v3.y, sk) + 1.f) * 0.5f);

			return materials[0]->opacity(uv, ray.time, worker, filter);
		}
	}

	return 0.f;
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

	float c = math::dot(transformation.rotation.v3.z, -wi);

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

	float c = math::dot(normal, -wi);

	if (two_sided) {
		c = std::abs(c);
	}

	if (c <= 0.f) {
		return 0.f;
	}

	float d = math::dot(normal, transformation.position);
	float denom = math::dot(normal, wi);
	float numer = math::dot(normal, p) - d;
	float t = -(numer / denom);

	float3 ws = p + t * wi; // ray.point(t);
	float3 k = ws - transformation.position;
	float l = math::dot(k, k);

	float radius = transformation.scale.x;

	if (l <= radius * radius) {
		float sl = t * t;
		return sl / (c * area);
	}

	return 0.f;
}

void Rectangle::sample(uint32_t /*part*/, const Transformation& /*transformation*/,
					   float3_p /*p*/, float2 /*uv*/, float /*area*/, Sample& /*sample*/) const {}

float Rectangle::pdf_uv(uint32_t /*part*/, const Transformation& /*transformation*/,
						float3_p /*p*/, float3_p /*wi*/, float /*area*/,
						float2& /*uv*/) const {
	return 1.f;
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

