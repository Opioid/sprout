#include "disk.hpp"
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

#include "shape_test.hpp"
#include "base/debug/assert.hpp"

namespace scene { namespace shape {

Disk::Disk() {
	aabb_.set_min_max(float3(-1.f, -1.f, -0.1f), float3(1.f, 1.f, 0.1f));
}

bool Disk::intersect(const Transformation& transformation, Ray& ray,
					 Node_stack& /*node_stack*/, Intersection& intersection) const {
	const float3& normal = transformation.rotation.r[2];
	float d = math::dot(normal, transformation.position);
	float denom = -math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) - d;
	float hit_t = numer / denom;

	if (hit_t > ray.min_t && hit_t < ray.max_t) {
		float3 p = ray.point(hit_t);
		float3 k = p - transformation.position;
		float l = math::dot(k, k);

		float radius = transformation.scale[0];

		if (l <= radius * radius) {
			intersection.epsilon = 5e-4f * hit_t;

			intersection.p = p;

			float3 t = -transformation.rotation.r[0];
			float3 b = -transformation.rotation.r[1];
			intersection.t = t;
			intersection.b = b;
			intersection.n = normal;
			intersection.geo_n = normal;

			float3 sk = k / radius;
			float uv_scale = 0.5f * transformation.scale[2];
			intersection.uv[0] = (math::dot(t, sk) + 1.f) * uv_scale;
			intersection.uv[1] = (math::dot(b, sk) + 1.f) * uv_scale;

			intersection.part = 0;

			SOFT_ASSERT(testing::check(intersection, transformation, ray));

			ray.max_t = hit_t;
			return true;
		}
	}

	return false;
}

bool Disk::intersect_p(const Transformation& transformation,
					   const Ray& ray, Node_stack& /*node_stack*/) const {
	const float3& normal = transformation.rotation.r[2];
	float d = math::dot(normal, transformation.position);
	float denom = -math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) - d;
	float hit_t = numer / denom;

	if (hit_t > ray.min_t && hit_t < ray.max_t) {
		float3 p = ray.point(hit_t);
		float3 k = p - transformation.position;
		float l = math::dot(k, k);

		float radius = transformation.scale[0];

		if (l <= radius * radius) {
			return true;
		}
	}

	return false;
}

float Disk::opacity(const Transformation& transformation, const Ray& ray,
					const material::Materials& materials,
					Worker& worker, Sampler_filter filter) const {
	const float3& normal = transformation.rotation.r[2];
	float d = math::dot(normal, transformation.position);
	float denom = -math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) - d;
	float hit_t = numer / denom;

	if (hit_t > ray.min_t && hit_t < ray.max_t) {
		float3 p = ray.point(hit_t);
		float3 k = p - transformation.position;
		float l = math::dot(k, k);

		float radius = transformation.scale[0];

		if (l <= radius * radius) {
			float3 sk = k / radius;
			float uv_scale = 0.5f * transformation.scale[2];
			float2 uv((-math::dot(transformation.rotation.r[0], sk) + 1.f) * uv_scale,
					  (-math::dot(transformation.rotation.r[1], sk) + 1.f) * uv_scale);

			return materials[0]->opacity(uv, ray.time, worker, filter);
		}
	}

	return 0.f;
}

float3 Disk::thin_absorption(const Transformation& transformation, const Ray& ray,
							 const material::Materials& materials,
							 Worker& worker, Sampler_filter filter) const {
	const float3& normal = transformation.rotation.r[2];
	float d = math::dot(normal, transformation.position);
	float denom = -math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) - d;
	float hit_t = numer / denom;

	if (hit_t > ray.min_t && hit_t < ray.max_t) {
		float3 p = ray.point(hit_t);
		float3 k = p - transformation.position;
		float l = math::dot(k, k);

		float radius = transformation.scale[0];

		if (l <= radius * radius) {
			float3 sk = k / radius;
			float uv_scale = 0.5f * transformation.scale[2];
			float2 uv((math::dot(transformation.rotation.r[0], sk) + 1.f) * uv_scale,
					  (math::dot(transformation.rotation.r[1], sk) + 1.f) * uv_scale);

			return materials[0]->thin_absorption(ray.direction, normal, uv,
												 ray.time, worker, filter);
		}
	}

	return float3(0.f);
}

void Disk::sample(uint32_t part, const Transformation& transformation,
				  const float3& p, const float3& /*n*/, float area, bool two_sided,
				  sampler::Sampler& sampler, uint32_t sampler_dimension,
				  Node_stack& node_stack, Sample& sample) const {
	Disk::sample(part, transformation, p, area, two_sided,
				 sampler, sampler_dimension, node_stack, sample);
}

void Disk::sample(uint32_t /*part*/, const Transformation& transformation,
				  const float3& p, float area, bool two_sided,
				  sampler::Sampler& sampler, uint32_t sampler_dimension,
				  Node_stack& /*node_stack*/, Sample& sample) const {
	float2 r2 = sampler.generate_sample_2D(sampler_dimension);
	float2 xy = math::sample_disk_concentric(r2);

	float3 ls = float3(xy, 0.f);
	float3 ws = transformation.position
			  + transformation.scale[0] * math::transform_vector(ls, transformation.rotation);

	float3 axis = ws - p;

	float sl = math::squared_length(axis);
	float t  = std::sqrt(sl);

	float3 wi = axis / t;

	float c = -math::dot(transformation.rotation.r[2], wi);

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

float Disk::pdf(const Ray& ray, const shape::Intersection& /*intersection*/,
				const Transformation& transformation,
				float area, bool two_sided, bool /*total_sphere*/) const {
	const float3 normal = transformation.rotation.r[2];

	float c = -math::dot(normal, ray.direction);

	if (two_sided) {
		c = std::abs(c);
	}

	float sl = ray.max_t * ray.max_t;
	return sl / (c * area);

}

void Disk::sample(uint32_t /*part*/, const Transformation& /*transformation*/,
				  const float3& /*p*/, float2 /*uv*/, float /*area*/, bool /*two_sided*/,
				  Sample& /*sample*/) const {}

float Disk::pdf_uv(const Ray& /*ray*/, const Intersection& /*intersection*/,
				   const Transformation& /*transformation*/,
				   float /*area*/, bool /*two_sided*/) const {
	return 0.f;
}

float Disk::uv_weight(float2 /*uv*/) const {
	return 1.f;
}

float Disk::area(uint32_t /*part*/, const float3& scale) const {
	return math::Pi * scale[0] * scale[0];
}

size_t Disk::num_bytes() const {
	return sizeof(*this);
}

}}
