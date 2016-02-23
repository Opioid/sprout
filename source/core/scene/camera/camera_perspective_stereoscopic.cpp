#include "camera_perspective_stereoscopic.hpp"
#include "rendering/sensor/sensor.hpp"
#include "sampler/camera_sample.hpp"
#include "scene/scene_ray.inl"
#include "base/math/math.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/ray.inl"
#include "base/math/sampling/sampling.inl"

namespace scene { namespace camera {

Perspective_stereoscopic::Perspective_stereoscopic(float interpupillary_distance,
												   math::int2 resolution, float ray_max_t,
												   float frame_duration, bool motion_blur, float fov) :
	Stereoscopic(interpupillary_distance, resolution, ray_max_t, frame_duration, motion_blur),
	fov_(fov) {
	math::float2 fr(resolution);
	float ratio = fr.x / fr.y;

	float z = ratio * math::Pi / fov_ * 0.5f;

	left_top_ = math::vec3(-ratio,  1.f, z);
	math::vec3 right_top	( ratio,  1.f, z);
	math::vec3 left_bottom(-ratio, -1.f, z);

	d_x_ = (right_top - left_top_)   / fr.x;
	d_y_ = (left_bottom - left_top_) / fr.y;

	view_bounds_[0] = math::Recti{math::int2(0, 0), resolution};
	view_bounds_[1] = math::Recti{math::int2(resolution.x, 0), math::int2(resolution.x * 2, resolution.y)};
}

uint32_t Perspective_stereoscopic::num_views() const {
	return 2;
}

math::int2 Perspective_stereoscopic::sensor_dimensions() const {
	return math::int2(resolution_.x * 2, resolution_.y);
}

math::Recti Perspective_stereoscopic::view_bounds(uint32_t view) const {
	return view_bounds_[view];
}

void Perspective_stereoscopic::update_focus(rendering::Worker& /*worker*/) {}

void Perspective_stereoscopic::generate_ray(const sampler::Camera_sample& sample, uint32_t view,
											scene::Ray& ray) const {
	math::float2 coordinates =  math::float2(sample.pixel) + sample.pixel_uv;

	math::vec3 direction = left_top_ + coordinates.x * d_x_ + coordinates.y * d_y_;

	entity::Composed_transformation temp;
	auto& transformation = transformation_at(sample.time, temp);

	ray.origin = math::transform_point(eye_offsets_[view], transformation.object_to_world);
	ray.set_direction(math::transform_vector(math::normalized(direction), transformation.object_to_world));
	ray.min_t = 0.f;
	ray.max_t = ray_max_t_;
	ray.time = sample.time;
	ray.depth = 0;
}

}}
