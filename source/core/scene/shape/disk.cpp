#include "disk.hpp"
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

#include "shape_test.hpp"
#include "base/debug/assert.hpp"

namespace scene { namespace shape {

Disk::Disk() {
	aabb_.set_min_max(float3(-1.f, -1.f, -0.1f), float3(1.f, 1.f, 0.1f));
}

bool Disk::intersect(const Transformation& transformation, Ray& ray,
					 Node_stack& /*node_stack*/, Intersection& intersection) const {
	float3_p normal = transformation.rotation.v3.z;
	float d = math::dot(normal, transformation.position);
	float denom = -math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) - d;
	float hit_t = numer / denom;

	if (hit_t > ray.min_t && hit_t < ray.max_t) {
		float3 p = ray.point(hit_t);
		float3 k = p - transformation.position;
		float l = math::dot(k, k);

		float radius = transformation.scale.x;

		if (l <= radius * radius) {
			intersection.epsilon = 5e-4f * hit_t;

			intersection.p = p;

			float3 t = -transformation.rotation.v3.x;
			float3 b = -transformation.rotation.v3.y;
			intersection.t = t;
			intersection.b = b;
			intersection.n = normal;
			intersection.geo_n = normal;

			float3 sk = k / radius;
			float uv_scale = 0.5f * transformation.scale.z;
			intersection.uv.x = (math::dot(t, sk) + 1.f) * uv_scale;
			intersection.uv.y = (math::dot(b, sk) + 1.f) * uv_scale;

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
	float3_p normal = transformation.rotation.v3.z;
	float d = math::dot(normal, transformation.position);
	float denom = -math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) - d;
	float hit_t = numer / denom;

	if (hit_t > ray.min_t && hit_t < ray.max_t) {
		float3 p = ray.point(hit_t);
		float3 k = p - transformation.position;
		float l = math::dot(k, k);

		float radius = transformation.scale.x;

		if (l <= radius * radius) {
			return true;
		}
	}

	return false;
}

float Disk::opacity(const Transformation& transformation, const Ray& ray,
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
		float l = math::dot(k, k);

		float radius = transformation.scale.x;

		if (l <= radius * radius) {
			float3 sk = k / radius;
			float uv_scale = 0.5f * transformation.scale.z;
			float2 uv((math::dot(transformation.rotation.v3.x, sk) + 1.f) * uv_scale,
					  (math::dot(transformation.rotation.v3.y, sk) + 1.f) * uv_scale);

			return materials[0]->opacity(uv, ray.time, worker, filter);
		}
	}

	return 0.f;
}

void Disk::sample(uint32_t part, const Transformation& transformation,
				  float3_p p, float3_p /*n*/, float area, bool two_sided,
				  sampler::Sampler& sampler, uint32_t sampler_dimension,
				  Node_stack& node_stack, Sample& sample) const {
	Disk::sample(part, transformation, p, area, two_sided,
				 sampler, sampler_dimension, node_stack, sample);
}

void Disk::sample(uint32_t /*part*/, const Transformation& transformation,
				  float3_p p, float area, bool two_sided,
				  sampler::Sampler& sampler, uint32_t sampler_dimension,
				  Node_stack& /*node_stack*/, Sample& sample) const {
	float2 r2 = sampler.generate_sample_2D(sampler_dimension);
	float2 xy = math::sample_disk_concentric(r2);

	float3 ls = float3(xy, 0.f);
	float3 ws = transformation.position
			  + transformation.scale.x * math::transform_vector(ls, transformation.rotation);

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

float Disk::pdf(uint32_t /*part*/, const Transformation& transformation,
				float3_p p, float3_p wi, float area, bool two_sided,
				bool /*total_sphere*/, Node_stack& /*node_stack*/) const {
	float3 normal = transformation.rotation.v3.z;

	float c = -math::dot(normal, wi);

	if (two_sided) {
		c = std::abs(c);
	}

	if (c <= 0.f) {
		return 0.f;
	}

	float d = math::dot(normal, transformation.position);
	float denom = -math::dot(normal, wi);
	float numer = math::dot(normal, p) - d;
	float hit_t = numer / denom;

	float3 ws = p + hit_t * wi; // ray.point(t);
	float3 k = ws - transformation.position;
	float l = math::dot(k, k);

	float radius = transformation.scale.x;

	if (l <= radius * radius) {
		float sl = hit_t * hit_t;
		return sl / (c * area);
	}

	return 0.f;
}

void Disk::sample(uint32_t /*part*/, const Transformation& /*transformation*/,
				  float3_p /*p*/, float2 /*uv*/, float /*area*/, bool /*two_sided*/,
				  Sample& /*sample*/) const {}

float Disk::pdf_uv(uint32_t /*part*/, const Transformation& /*transformation*/,
				   float3_p /*p*/, float3_p /*wi*/, float /*area*/, bool /*two_sided*/,
				   float2& /*uv*/) const {
	return 1.f;
}

float Disk::uv_weight(float2 /*uv*/) const {
	return 1.f;
}

float Disk::area(uint32_t /*part*/, float3_p scale) const {
	return math::Pi * scale.x * scale.x;
}

size_t Disk::num_bytes() const {
	return sizeof(*this);
}

}}
