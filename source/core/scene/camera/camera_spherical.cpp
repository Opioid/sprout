#include "camera_spherical.hpp"
#include "rendering/sensor/sensor.hpp"
#include "sampler/camera_sample.hpp"
#include "base/math/math.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/ray.inl"
#include "base/math/sampling/sampling.inl"

namespace scene { namespace camera {

Spherical::Spherical(math::uint2 resolution, float ray_max_t, float frame_duration, bool motion_blur) :
	Camera(resolution, ray_max_t, frame_duration, motion_blur) {
	math::float2 fr(resolution);
	d_x_ = 1.f / fr.x;
	d_y_ = 1.f / fr.y;
}

math::uint2 Spherical::sensor_dimensions() const {
	return resolution_;
}

void Spherical::update_focus(rendering::Worker& /*worker*/) {}

void Spherical::generate_ray(const sampler::Camera_sample& sample, math::Oray& ray) const {
	float x = d_x_ * sample.coordinates.x;
	float y = d_y_ * sample.coordinates.y;

	float phi   = (-x + 0.75f) * 2.f * math::Pi;
	float theta = y * math::Pi;

	float sin_theta = std::sin(theta);
	float cos_theta = std::cos(theta);
	float sin_phi   = std::sin(phi);
	float cos_phi   = std::cos(phi);

	math::float3 dir(sin_theta * cos_phi, cos_theta, sin_theta * sin_phi);

	entity::Composed_transformation transformation;
	transformation_at(ray.time, transformation);
	ray.origin = transformation.position;
	ray.set_direction(math::transform_vector(transformation.rotation, dir));
	ray.min_t = 0.f;
	ray.max_t = ray_max_t_;
	ray.depth = 0;
}

}}
