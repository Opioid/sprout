#include "inverse_sphere.hpp"
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

Inverse_sphere::Inverse_sphere() {
	aabb_.set_min_max(float3(-1.f, -1.f, -1.f), float3(1.f, 1.f, 1.f));
}

bool Inverse_sphere::intersect(const Transformation& transformation,
							   Ray& ray, Node_stack& /*node_stack*/,
							   Intersection& intersection) const {
	float3 v = ray.origin - transformation.position;
	float b = -dot(v, ray.direction);
	float radius = transformation.scale.x;
	float det = (b * b) - dot(v, v) + (radius * radius);

	if (det > 0.f) {
		float dist = std::sqrt(det);
		float t0 = b - dist;

		if (t0 > ray.min_t && t0 < ray.max_t) {
			intersection.epsilon = 5e-4f * t0;

			intersection.p = ray.point(t0);
			intersection.n = math::normalized(transformation.position - intersection.p);
			math::coordinate_system(intersection.n, intersection.t, intersection.b);
			intersection.geo_n = intersection.n;

			float3 xyz = math::transform_vector_transposed(-intersection.n,
															transformation.rotation);
			intersection.uv = float2(std::atan2(xyz.x, xyz.z) * math::Pi_inv * 0.5f + 0.5f,
									 std::acos(xyz.y) * math::Pi_inv);

			intersection.part = 0;

			ray.max_t = t0;
			return true;
		}

		float t1 = b + dist;

		if (t1 > ray.min_t && t1 < ray.max_t) {
			intersection.epsilon = 5e-4f * t1;

			intersection.p = ray.point(t1);
			intersection.n = math::normalized(transformation.position - intersection.p);
			math::coordinate_system(intersection.n, intersection.t, intersection.b);
			intersection.geo_n = intersection.n;

			float3 xyz = math::transform_vector_transposed(-intersection.n,
															transformation.rotation);
			intersection.uv = float2(std::atan2(xyz.x, xyz.z) * math::Pi_inv * 0.5f + 0.5f,
									 std::acos(xyz.y) * math::Pi_inv);

			intersection.part = 0;

			ray.max_t = t1;
			return true;
		}
	}

	return false;
}

bool Inverse_sphere::intersect_p(const Transformation& transformation,
								 const Ray& ray, Node_stack& /*node_stack*/) const {
	float3 v = ray.origin - transformation.position;
	float b = -dot(v, ray.direction);
	float radius = transformation.scale.x;
	float det = (b * b) - dot(v, v) + (radius * radius);

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

float Inverse_sphere::opacity(const Transformation& transformation,
							  const Ray& ray, const material::Materials& /*materials*/,
							  Worker& worker, Sampler_filter /*filter*/) const {
	return intersect_p(transformation, ray, worker.node_stack()) ? 1.f : 0.f;
}

void Inverse_sphere::sample(uint32_t /*part*/, const Transformation& transformation,
							float3_p p, float3_p n, float /*area*/, bool /*two_sided*/,
							sampler::Sampler& sampler, uint32_t sampler_dimension,
							Node_stack& /*node_stack*/, Sample& sample) const {
	float3 dir = n;

	math::Ray ray;
	ray.origin = p;
	ray.set_direction(dir);
	ray.min_t  = 0.1f;
	ray.max_t  = 10000.f;

	Intersection intersection;

	float3 v = ray.origin - transformation.position;
	float b = -dot(v, ray.direction);
	float radius = transformation.scale.x;
	float det = (b * b) - dot(v, v) + (radius * radius);

	if (det > 0.f) {
		float dist = std::sqrt(det);
		float t1 = b + dist;

		if (t1 > ray.min_t && t1 < ray.max_t) {

			intersection.p = ray.point(t1);
//			intersection.n = (transformation.position - intersection.p) / radius;
			intersection.n = math::normalized(p - intersection.p);

			float3 xyz = math::transform_vector_transposed(-intersection.n,
														   transformation.rotation);
			intersection.uv = float2(std::atan2(xyz.x, xyz.z) * math::Pi_inv * 0.5f + 0.5f,
									 std::acos(xyz.y) * math::Pi_inv);
		}
	}

	float3 axis = p - intersection.p;
	float axis_squared_length = math::squared_length(axis);

	float radius_square = transformation.scale.x * transformation.scale.x;
	float sin_theta_max2 = radius_square / axis_squared_length;
	float cos_theta_max  = std::sqrt(std::max(0.f, 1.f - sin_theta_max2));
	cos_theta_max = std::min(0.99999995f, cos_theta_max);

	float axis_length = std::sqrt(axis_squared_length);
	float3 z = axis / axis_length;
	float3 x, y;
	math::coordinate_system(z, x, y);

	float2 r2 = sampler.generate_sample_2D(sampler_dimension);
	float3 tdir = -math::sample_oriented_cone_uniform(r2, cos_theta_max, x, y, z);

	sample.wi = tdir;
	sample.t = axis_length - transformation.scale.x; // this is not accurate
	sample.pdf = math::cone_pdf_uniform(cos_theta_max);

	float3 xyz = math::transform_vector_transposed(tdir, transformation.rotation);
	sample.uv = float2(std::atan2(xyz.x, xyz.z) * math::Pi_inv * 0.5f + 0.5f,
					   std::acos(xyz.y) * math::Pi_inv);
}

void Inverse_sphere::sample(uint32_t /*part*/, const Transformation& /*transformation*/,
							float3_p /*p*/, float /*area*/, bool /*two_sided*/,
							sampler::Sampler& /*sampler*/, uint32_t /*sampler_dimension*/,
							Node_stack& /*node_stack*/, Sample& /*sample*/) const {
	// TODO
}

float Inverse_sphere::pdf(uint32_t /*part*/, const Transformation& transformation,
						  float3_p p, float3_p /*wi*/, float /*area*/,
						  bool /*two_sided*/, bool /*total_sphere*/,
						  Node_stack& /*node_stack*/) const {
	float3 axis = transformation.position - p;
	float axis_squared_length = math::squared_length(axis);

	float radius_square  = transformation.scale.x * transformation.scale.x;
	float sin_theta_max2 = radius_square / axis_squared_length;
	float cos_theta_max  = std::sqrt(std::max(0.f, 1.f - sin_theta_max2));

	return math::cone_pdf_uniform(cos_theta_max);
}

void Inverse_sphere::sample(uint32_t /*part*/, const Transformation& transformation,
							float3_p p, float2 uv, float area, Sample& sample) const {
	float phi   = (uv.x + 0.75f) * 2.f * math::Pi;
	float theta = uv.y * math::Pi;

	float sin_theta = std::sin(theta);
	float cos_theta = std::cos(theta);
	float sin_phi   = std::sin(phi);
	float cos_phi   = std::cos(phi);

	float3 ls(sin_theta * cos_phi, cos_theta, sin_theta * sin_phi);
	float3 ws = math::transform_point(ls, transformation.object_to_world);

	float3 axis = ws - p;
	float sl = math::squared_length(axis);
	float d = std::sqrt(sl);

	float3 dir = axis / d;

	float3 wn = math::normalized(transformation.position - ws);

	float c = math::dot(wn, -dir);

	if (c <= 0.f) {
		sample.pdf = 0.f;
	} else {
		sample.wi = dir;
		sample.uv = uv;
		sample.t  = d;
		// sin_theta because of the uv weight
		sample.pdf = sl / (c * area * sin_theta);
	}
}

float Inverse_sphere::pdf_uv(uint32_t /*part*/, const Transformation& transformation,
							 float3_p p, float3_p wi, float /*area*/, float2& uv) const {
	float3 v = p - transformation.position;
	float b = -math::dot(v, wi);
	float radius = transformation.scale.x;
	float det = (b * b) - math::dot(v, v) + (radius * radius);

	if (det > 0.f) {
		float dist = std::sqrt(det);

		float t = b + dist;
		float3 hit = p + t * wi;
		float3 dir = math::normalized(hit - transformation.position);

		float3 xyz = math::transform_vector_transposed(dir, transformation.rotation);
		uv.x = std::atan2(xyz.x, xyz.z) * math::Pi_inv * 0.5f + 0.5f;
		uv.y = std::acos(xyz.y) * math::Pi_inv;

		// sin_theta because of the uv weight
		float sin_theta = std::sqrt(1.f - xyz.y * xyz.y);

		return 1.f / (4.f * math::Pi * sin_theta);
	}

	return 0.f;
}

float Inverse_sphere::uv_weight(float2 uv) const {
	float sin_theta = std::sin(uv.y * math::Pi);

	if (0.f == sin_theta) {
		// this case never seemed to be an issue?!
		return 0.f;
	}

	return 1.f / sin_theta;
}

float Inverse_sphere::area(uint32_t /*part*/, float3_p scale) const {
	return 4.f * math::Pi * scale.x * scale.x;
}

size_t Inverse_sphere::num_bytes() const {
	return sizeof(*this);
}

}}
