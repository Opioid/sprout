#include "canopy.hpp"
#include "shape_sample.hpp"
#include "shape_intersection.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/entity/composed_transformation.hpp"
#include "sampler/sampler.hpp"
#include "base/math/mapping.inl"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/math/matrix3x3.inl"
#include "base/math/sampling/sampling.inl"

#include "shape_test.hpp"
#include "base/debug/assert.hpp"

namespace scene { namespace shape {

Canopy::Canopy() {
	aabb_.set_min_max(float3::identity(), float3::identity());
}

bool Canopy::intersect(const Transformation& transformation, Ray& ray,
					   Node_stack& /*node_stack*/, Intersection& intersection) const {
	if (ray.max_t >= Ray_max_t) {
		if (math::dot(ray.direction, transformation.rotation.r[2]) < 0.f) {
			return false;
		}

		intersection.epsilon = 5e-4f;

		intersection.p = ray.point(Ray_max_t);
		intersection.t = transformation.rotation.r[0];
		intersection.b = transformation.rotation.r[1];

		float3 n = -ray.direction;
		intersection.n = n;
		intersection.geo_n = n;
		intersection.part = 0;

		// paraboloid, so doesn't match hemispherical camera
		float3 xyz = math::transform_vector_transposed(ray.direction, transformation.rotation);
		xyz = math::normalized(xyz);
		float2 disk = math::hemisphere_to_disk_equidistant(xyz);
		intersection.uv[0] = 0.5f * disk[0] + 0.5f;
		intersection.uv[1] = 0.5f * disk[1] + 0.5f;

		ray.max_t = Ray_max_t;
		return true;
	}

	return false;
}

bool Canopy::intersect_p(const Transformation& /*transformation*/,
						 const Ray& /*ray*/, Node_stack& /*node_stack*/) const {
	// Implementation for this is not really needed, so just skip it
	return false;
}

float Canopy::opacity(const Transformation& /*transformation*/,
					  const Ray& /*ray*/, const material::Materials& /*materials*/,
					  Worker& /*worker*/, Sampler_filter /*filter*/) const {
	// Implementation for this is not really needed, so just skip it
	return 0.f;
}

float3 Canopy::thin_absorption(const Transformation& /*transformation*/,
							   const Ray& /*ray*/, const material::Materials& /*materials*/,
							   Worker& /*worker*/, Sampler_filter /*filter*/) const {
	// Implementation for this is not really needed, so just skip it
	return float3(0.f);
}

void Canopy::sample(uint32_t part, const Transformation& transformation,
					float3_p p, float3_p /*n*/, float area, bool two_sided,
					sampler::Sampler& sampler, uint32_t sampler_dimension,
					Node_stack& node_stack, Sample& sample) const {
	Canopy::sample(part, transformation, p, area, two_sided,
				   sampler, sampler_dimension, node_stack, sample);
}

void Canopy::sample(uint32_t /*part*/, const Transformation& transformation,
					float3_p /*p*/, float /*area*/, bool /*two_sided*/,
					sampler::Sampler& sampler, uint32_t sampler_dimension,
					Node_stack& /*node_stack*/, Sample& sample) const {
	float2 uv = sampler.generate_sample_2D(sampler_dimension);
	float3 dir = math::sample_oriented_hemisphere_uniform(uv, transformation.rotation);

	sample.wi = dir;

	float3 xyz = math::transform_vector_transposed(dir, transformation.rotation);
	xyz = math::normalized(xyz);
	float2 disk = math::hemisphere_to_disk_equidistant(xyz);
	sample.uv[0] = 0.5f * disk[0] + 0.5f;
	sample.uv[1] = 0.5f * disk[1] + 0.5f;

	sample.t   = Ray_max_t;
	sample.pdf = 1.f / (2.f * math::Pi);

	SOFT_ASSERT(testing::check(sample));
}

float Canopy::pdf(uint32_t /*part*/, const Transformation& /*transformation*/,
				  float3_p /*p*/, float3_p /*wi*/, float /*area*/, bool /*two_sided*/,
				  bool /*total_sphere*/, Node_stack& /*node_stack*/) const {
	return 1.f / (2.f * math::Pi);
}

void Canopy::sample(uint32_t /*part*/, const Transformation& transformation,
					float3_p /*p*/, float2 uv, float /*area*/, bool /*two_sided*/,
					Sample& sample) const {
	float2 disk(2.f * uv[0] - 1.f, 2.f * uv[1] - 1.f);

	float z = math::dot(disk, disk);
	if (z > 1.f) {
		sample.pdf = 0.f;
		return;
	}

	float3 dir = math::disk_to_hemisphere_equidistant(disk);

	sample.wi = math::transform_vector(dir, transformation.rotation);
	sample.uv = uv;
	sample.t  = Ray_max_t;



//	float r = std::sqrt(disk.x * disk.x + disk.y * disk.y);

//	// Equidistant projection
//	float longitude = std::atan2(-uv.y, uv.x);
//	float colatitude = r * math::Pi_div_2;

//	float sin_col = std::sin(colatitude);
//	float cos_col = std::cos(colatitude);
//	float sin_lon = std::sin(longitude);
//	float cos_lon = std::cos(longitude);

	sample.pdf = 1.f / (2.f * math::Pi /** std::cos(std::asin(r))*/);


}

float Canopy::pdf_uv(uint32_t /*part*/, const Transformation& transformation,
					 float3_p /*p*/, float3_p wi, float /*area*/, bool /*two_sided*/,
					 float2& uv) const {
	float3 xyz = math::transform_vector_transposed(wi, transformation.rotation);
	xyz = math::normalized(xyz);
	float2 disk = math::hemisphere_to_disk_equidistant(xyz);
	uv[0] = 0.5f * disk[0] + 0.5f;
	uv[1] = 0.5f * disk[1] + 0.5f;


//	float r = std::sqrt(disk.x * disk.x + disk.y * disk.y);

//	// Equidistant projection
//	float longitude = std::atan2(-uv.y, uv.x);
//	float colatitude = r * math::Pi_div_2;

//	float sin_col = std::sin(colatitude);
//	float cos_col = std::cos(colatitude);
//	float sin_lon = std::sin(longitude);
//	float cos_lon = std::cos(longitude);

	return 1.f / (2.f * math::Pi /** std::cos(std::asin(r))*/);
}

float Canopy::uv_weight(float2 uv) const {
	float2 disk(2.f * uv[0] - 1.f, 2.f * uv[1] - 1.f);

	float z = math::dot(disk, disk);
	if (z > 1.f) {
		return 0.f;
	}


//	float r = std::sqrt(disk.x * disk.x + disk.y * disk.y);

//	// Equidistant projection
//	float longitude = std::atan2(-uv.y, uv.x);
//	float colatitude = r * math::Pi_div_2;

//	float sin_col = std::sin(colatitude);
//	float cos_col = std::cos(colatitude);
//	float sin_lon = std::sin(longitude);
//	float cos_lon = std::cos(longitude);

//	return ( std::cos(std::asin(r))  );

	return 1.f;
}

float Canopy::area(uint32_t /*part*/, float3_p /*scale*/) const {
	return 2.f * math::Pi;
}

bool Canopy::is_finite() const {
	return false;
}

size_t Canopy::num_bytes() const {
	return sizeof(*this);
}

}}
