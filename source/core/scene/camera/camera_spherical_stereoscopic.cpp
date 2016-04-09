#include "camera_spherical_stereoscopic.hpp"
#include "rendering/sensor/sensor.hpp"
#include "sampler/camera_sample.hpp"
#include "scene/scene_ray.inl"
#include "base/math/math.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/sampling/sampling.inl"

namespace scene { namespace camera {

Spherical_stereoscopic::Spherical_stereoscopic(float interpupillary_distance,
											   math::int2 resolution, float ray_max_t,
											   float frame_duration, bool motion_blur) :
	Stereoscopic(interpupillary_distance, resolution, ray_max_t, frame_duration, motion_blur) {
	math::float2 fr(resolution);
	d_x_ = 1.f / fr.x;
	d_y_ = 1.f / fr.y;

	view_bounds_[0] = math::Recti{math::int2(0, 0), resolution};
	view_bounds_[1] = math::Recti{math::int2(resolution.x, 0), math::int2(resolution.x * 2, resolution.y)};
}

uint32_t Spherical_stereoscopic::num_views() const {
	return 2;
}

math::int2 Spherical_stereoscopic::sensor_dimensions() const {
	return math::int2(resolution_.x * 2, resolution_.y);
}

math::Recti Spherical_stereoscopic::view_bounds(uint32_t view) const {
	return view_bounds_[view];
}

void Spherical_stereoscopic::update_focus(rendering::Worker& /*worker*/) {}

bool Spherical_stereoscopic::generate_ray(const sampler::Camera_sample& sample, uint32_t view, scene::Ray& ray) const {
	math::float2 coordinates =  math::float2(sample.pixel) + sample.pixel_uv;

	float x = d_x_ * coordinates.x;
	float y = d_y_ * coordinates.y;

	float phi   = (-x + 0.75f) * 2.f * math::Pi;
	float theta = y * math::Pi;

	float sin_theta = std::sin(theta);
	float cos_theta = std::cos(theta);
	float sin_phi   = std::sin(phi);
	float cos_phi   = std::cos(phi);

	math::float3 dir(sin_theta * cos_phi, cos_theta, sin_theta * sin_phi);

	entity::Composed_transformation temp;
	auto& transformation = transformation_at(sample.time, temp);

	math::float3x3 rotation;
	math::set_rotation_y(rotation, (x - 0.5f) * 2.f * math::Pi);
	math::float3 eye_pos = rotation * eye_offsets_[view];

	ray.origin = math::transform_point(eye_pos, transformation.object_to_world);
	ray.set_direction(math::transform_vector(dir, transformation.rotation));
	ray.min_t = 0.f;
	ray.max_t = ray_max_t_;
	ray.time  = sample.time;
	ray.depth = 0;

	return true;
}

}}
