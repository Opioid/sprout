#include "camera_hemispherical.hpp"
#include "rendering/sensor/sensor.hpp"
#include "sampler/camera_sample.hpp"
#include "scene/scene_ray.inl"
#include "base/math/mapping.inl"
#include "base/math/math.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/sampling/sampling.inl"

namespace scene { namespace camera {

Hemispherical::Hemispherical(int2 resolution, float ray_max_t) :
	Camera(resolution, ray_max_t) {
	float2 fr(resolution);
	d_x_ = 1.f / fr.x;
	d_y_ = 1.f / fr.y;
}

uint32_t Hemispherical::num_views() const {
	return 1;
}

int2 Hemispherical::sensor_dimensions() const {
	return resolution_;
}

math::Recti Hemispherical::view_bounds(uint32_t /*view*/) const {
	return math::Recti{int2(0, 0), resolution_};
}

float Hemispherical::pixel_solid_angle() const {
	return 1.f;
}

void Hemispherical::update(rendering::Worker& /*worker*/) {}

bool Hemispherical::generate_ray(const sampler::Camera_sample& sample,
								 uint32_t /*view*/, scene::Ray& ray) const {
	float2 coordinates = float2(sample.pixel) + sample.pixel_uv;

	float x = d_x_ * coordinates.x;
	float y = d_y_ * coordinates.y;

	x = 2.f * x - 1.f;
	y = 2.f * y - 1.f;

	float z = x * x + y * y;
	if (z > 1.f) {
		return false;
	}

	float3 dir = math::disk_to_hemisphere_equidistant(float2(x, y));

	// paraboloid
//	float3 dir = math::normalized(float3(x, -y, 0.5f - 0.5f * z));

//	float3 dir = math::normalized(float3(x, -y, 1.f - std::sqrt(z)));

//	float3 dir = math::normalized(float3(x, -y,
//										std::sqrt(std::max(0.f, 1.f - x * x - y * y))));

	entity::Composed_transformation temp;
	auto& transformation = transformation_at(sample.time, temp);

	ray.origin = transformation.position;
	ray.set_direction(math::transform_vector(dir, transformation.rotation));
	ray.min_t = 0.f;
	ray.max_t = ray_max_t_;
	ray.time  = sample.time;
	ray.depth = 0;

	return true;
}

void Hemispherical::set_parameter(const std::string& /*name*/, const json::Value& /*value*/) {}

}}
