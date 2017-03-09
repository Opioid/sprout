#include "camera_spherical.hpp"
#include "rendering/sensor/sensor.hpp"
#include "sampler/camera_sample.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.hpp"
#include "base/math/math.hpp"
#include "base/math/vector4.inl"
#include "base/math/matrix4x4.inl"
#include "base/math/sampling/sampling.inl"

namespace scene { namespace camera {

Spherical::Spherical(int2 resolution) :
	Camera(resolution) {
	float2 fr(resolution);
	d_x_ = 1.f / fr[0];
	d_y_ = 1.f / fr[1];
}

uint32_t Spherical::num_views() const {
	return 1;
}

int2 Spherical::sensor_dimensions() const {
	return resolution_;
}

int4 Spherical::view_bounds(uint32_t /*view*/) const {
    return int4(int2(0, 0), resolution_ - int2(1, 1));
}

float Spherical::pixel_solid_angle() const {
	return 1.f;
}

void Spherical::update(rendering::Worker& /*worker*/) {}

bool Spherical::generate_ray(const sampler::Camera_sample& sample,
							 uint32_t /*view*/, scene::Ray& ray) const {
	float2 coordinates = float2(sample.pixel) + sample.pixel_uv;

	float x = d_x_ * coordinates[0];
	float y = d_y_ * coordinates[1];

	float phi   = (x - 0.5f) * (2.f * math::Pi);
	float theta = y * math::Pi;

	float sin_phi   = std::sin(phi);
	float cos_phi   = std::cos(phi);
	float sin_theta = std::sin(theta);
	float cos_theta = std::cos(theta);

	float3 dir(sin_phi * sin_theta, cos_theta, cos_phi * sin_theta);

	entity::Composed_transformation temp;
	auto& transformation = transformation_at(sample.time, temp);

	ray.origin = transformation.position;
	ray.set_direction(math::transform_vector(dir, transformation.rotation));
	ray.min_t = 0.f;
	ray.max_t = Ray_max_t;
	ray.time  = sample.time;
	ray.depth = 0;

	return true;
}

void Spherical::set_parameter(const std::string& /*name*/, const json::Value& /*value*/) {}

}}
