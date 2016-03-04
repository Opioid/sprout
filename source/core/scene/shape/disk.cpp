#include "disk.hpp"
#include "shape_sample.hpp"
#include "geometry/shape_intersection.hpp"
#include "scene/scene_worker.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "sampler/sampler.hpp"
#include "base/math/sampling/sampling.inl"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/ray.inl"
#include "base/math/bounding/aabb.inl"

namespace scene { namespace shape {

Disk::Disk() {
	aabb_.set_min_max(math::float3(-1.f, -1.f, -0.1f), math::float3(1.f, 1.f, 0.1f));
}

bool Disk::intersect(const entity::Composed_transformation& transformation, math::Oray& ray,
					 Node_stack& /*node_stack*/, Intersection& intersection) const {
	const math::float3& normal = transformation.rotation.z3;
	float d = -math::dot(normal, transformation.position);
	float denom = math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) + d;
	float t = -(numer / denom);

	if (t > ray.min_t && t < ray.max_t) {
		math::float3 p = ray.point(t);
		math::float3 k = p - transformation.position;
		float l = math::dot(k, k);

		float radius = transformation.scale.x;

		if (l <= radius * radius) {
			intersection.epsilon = 5e-4f * t;

			intersection.p = p;
			intersection.t = -transformation.rotation.x3;
			intersection.b = -transformation.rotation.y3;
			intersection.n = normal;
			intersection.geo_n = normal;
			math::float3 sk = k / radius;

			intersection.uv.x = (math::dot(intersection.t, sk) + 1.f) * 0.5f * transformation.scale.z;
			intersection.uv.y = (math::dot(intersection.b, sk) + 1.f) * 0.5f * transformation.scale.z;

			intersection.part = 0;

			ray.max_t = t;
			return true;
		}
	}

	return false;
}

bool Disk::intersect_p(const entity::Composed_transformation& transformation, const math::Oray& ray,
					   Node_stack& /*node_stack*/) const {
	const math::float3& normal = transformation.rotation.z3;
	float d = -math::dot(normal, transformation.position);
	float denom = math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) + d;
	float t = -(numer / denom);

	if (t > ray.min_t && t < ray.max_t) {
		math::float3 p = ray.point(t);
		math::float3 k = p - transformation.position;
		float l = math::dot(k, k);

		float radius = transformation.scale.x;

		if (l <= radius * radius) {
			return true;
		}
	}

	return false;
}

float Disk::opacity(const entity::Composed_transformation& transformation, const math::Oray& ray, float time,
					const material::Materials& materials,
					Worker& worker, material::Sampler_settings::Filter filter) const {
	const math::float3& normal = transformation.rotation.z3;
	float d = -math::dot(normal, transformation.position);
	float denom = math::dot(normal, ray.direction);
	float numer = math::dot(normal, ray.origin) + d;
	float t = -(numer / denom);

	if (t > ray.min_t && t < ray.max_t) {
		math::float3 p = ray.point(t);
		math::float3 k = p - transformation.position;
		float l = math::dot(k, k);

		float radius = transformation.scale.x;

		if (l <= radius * radius) {
			math::float3 sk = k / radius;
			math::float2 uv((math::dot(transformation.rotation.x3, sk) + 1.f) * 0.5f,
							(math::dot(transformation.rotation.y3, sk) + 1.f) * 0.5f);

			return materials[0]->opacity(uv, time, worker, filter);
		}
	}

	return 0.f;
}

void Disk::sample(uint32_t part, const entity::Composed_transformation& transformation, float area,
				  const math::float3& p, const math::float3& /*n*/, bool two_sided,
				  sampler::Sampler& sampler, Node_stack& node_stack, Sample& sample) const {
	Disk::sample(part, transformation, area, p, two_sided, sampler, node_stack, sample);
}

void Disk::sample(uint32_t /*part*/, const entity::Composed_transformation& transformation, float area,
				  const math::float3& p, bool two_sided,
				  sampler::Sampler& sampler, Node_stack& /*node_stack*/, Sample& sample) const {
	math::float2 r2 = sampler.generate_sample_2D();
	math::float2 xy = math::sample_disk_concentric(r2);

	math::float3 ls = math::float3(xy, 0.f);
	math::float3 ws = transformation.position + transformation.scale.x * math::transform_vector(ls, transformation.rotation);

	math::float3 axis = ws - p;

	math::float3 wi = math::normalized(axis);

	float c = math::dot(transformation.rotation.z3, -wi);

	if (two_sided) {
		c = std::abs(c);
	}

	if (c <= 0.f) {
		sample.pdf = 0.f;
	} else {
		sample.wi = wi;
		float sl = math::squared_length(axis);
		sample.t = std::sqrt(sl);
		sample.pdf = sl / (c * area);
	}
}

void Disk::sample(uint32_t /*part*/, const entity::Composed_transformation& /*transformation*/, float /*area*/,
				  const math::float3& /*p*/, math::float2 /*uv*/, Sample& /*sample*/) const {}

void Disk::sample(uint32_t /*part*/, const entity::Composed_transformation& /*transformation*/, float /*area*/,
				  const math::float3& /*p*/, const math::float3& /*wi*/, Sample& /*sample*/) const {}

float Disk::pdf(uint32_t /*part*/, const entity::Composed_transformation& transformation, float area,
				const math::float3& p, const math::float3& wi, bool two_sided, bool /*total_sphere*/,
				Node_stack& /*node_stack*/) const {
	math::float3 normal = transformation.rotation.z3;

	float c = math::dot(normal, -wi);

	if (two_sided) {
		c = std::abs(c);
	}

	if (c <= 0.f) {
		return 0.f;
	}

	float d = -math::dot(normal, transformation.position);
	float denom = math::dot(normal, wi);
	float numer = math::dot(normal, p) + d;
	float t = -(numer / denom);

	math::float3 ws = p + t * wi; // ray.point(t);
	math::float3 k = ws - transformation.position;
	float l = math::dot(k, k);

	float radius = transformation.scale.x;

	if (l <= radius * radius) {
		float sl = t * t;
		return sl / (c * area);
	}

	return 0.f;
}

float Disk::area(uint32_t /*part*/, const math::float3& scale) const {
	return math::Pi * scale.x * scale.x;
}

}}
