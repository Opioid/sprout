#include "disk.hpp"
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

Disk::Disk() {
	aabb_.set_min_max(math::float3(-1.f, -1.f, -1.f), math::float3(1.f, 1.f, 1.f));
}

bool Disk::intersect(const entity::Composed_transformation& transformation, math::Oray& ray,
					 const math::float2& /*bounds*/, Node_stack& /*node_stack*/,
					 Intersection& intersection) const {
	const math::float3& normal = transformation.rotation.z;
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
			intersection.t = transformation.rotation.x;
			intersection.b = transformation.rotation.y;
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
					   const math::float2& /*bounds*/, Node_stack& /*node_stack*/) const {
	const math::float3& normal = transformation.rotation.z;
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

float Disk::opacity(const entity::Composed_transformation& transformation, const math::Oray& ray,
					const math::float2& /*bounds*/, Node_stack& /*node_stack*/,
					const material::Materials& materials,
					const image::texture::sampler::Sampler_2D& sampler) const {
	const math::float3& normal = transformation.rotation.z;
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
			math::float2 uv((math::dot(transformation.rotation.x, sk) + 1.f) * 0.5f,
							(math::dot(transformation.rotation.y, sk) + 1.f) * 0.5f);

			return materials[0]->opacity(uv, sampler);
		}
	}

	return 0.f;
}

void Disk::sample(uint32_t /*part*/, const entity::Composed_transformation& transformation, float area,
				  const math::float3& p, const math::float3& /*n*/, bool /*restrict_to_hemisphere*/,
				  sampler::Sampler& sampler, Sample& sample) const {
	math::float2 r2 = sampler.generate_sample_2D();
	math::float2 xy = math::sample_disk_concentric(r2);

	math::float3 ls = math::float3(xy, 0.f);
	math::float3 ws = transformation.position + transformation.scale.x * math::transform_vector(transformation.rotation, ls);

	math::float3 axis = ws - p;

	sample.wi = math::normalized(axis);

	float c = math::dot(transformation.rotation.z, -sample.wi);
	if (c <= 0.f) {
		sample.pdf = 0.f;
	} else {
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
				const math::float3& p, const math::float3& wi, bool /*restrict_to_hemisphere*/) const {
	const math::float3& normal = transformation.rotation.z;

	float c = math::dot(normal, -wi);
	if (c <= 0.f) {
		return 0.f;
	}

	float d = -math::dot(normal, transformation.position);
	float denom = math::dot(normal, wi);
	float numer = math::dot(normal, p) + d;
	float t = -(numer / denom);

	math::float3 ws = p + t * wi; // ray.point(t);

	math::float3 axis = ws - p;

	float sl = math::squared_length(axis);

	return sl / (c * area);
}

float Disk::area(uint32_t /*part*/, const math::float3& scale) const {
	return math::Pi * scale.x * scale.x;
}

}}

