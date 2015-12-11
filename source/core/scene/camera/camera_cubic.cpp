#include "camera_cubic.hpp"
#include "rendering/sensor/sensor.hpp"
#include "sampler/camera_sample.hpp"
#include "base/math/math.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/ray.inl"
#include "base/math/sampling/sampling.inl"

namespace scene { namespace camera {

Cubic::Cubic(Layout layout, math::uint2 resolution, float ray_max_t, float frame_duration, bool motion_blur) :
	Camera(math::uint2(resolution.x, resolution.x), ray_max_t, frame_duration, motion_blur) {
	float f = static_cast<float>(resolution.x);

	left_top_ = math::float3(-1.f,  1.f, 1.f);
	math::float3 right_top	( 1.f,  1.f, 1.f);
	math::float3 left_bottom(-1.f, -1.f, 1.f);

	d_x_ = (right_top - left_top_)   / f;
	d_y_ = (left_bottom - left_top_) / f;

	if (Layout::xmxymyzmz == layout) {
		view_offsets_[0] = math::uint2(0, 0);
		view_offsets_[1] = math::uint2(resolution.x, 0);
		view_offsets_[2] = math::uint2(resolution.x * 2, 0);
		view_offsets_[3] = math::uint2(resolution.x * 3, 0);
		view_offsets_[4] = math::uint2(resolution.x * 4, 0);
		view_offsets_[5] = math::uint2(resolution.x * 5, 0);

		sensor_dimensions_ = math::uint2(resolution_.x * 6, resolution_.y);
	} else if (Layout::xmxy_myzmz == layout) {
		view_offsets_[0] = math::uint2(0, 0);
		view_offsets_[1] = math::uint2(resolution.x, 0);
		view_offsets_[2] = math::uint2(resolution.x * 2, 0);
		view_offsets_[3] = math::uint2(0, resolution.x);
		view_offsets_[4] = math::uint2(resolution.x, resolution.x);
		view_offsets_[5] = math::uint2(resolution.x * 2, resolution.x);

		sensor_dimensions_ = math::uint2(resolution_.x * 3, resolution_.y * 2);
	}

	math::set_rotation_y(view_rotations_[0], math::degrees_to_radians(-90.f));
	math::set_rotation_y(view_rotations_[1], math::degrees_to_radians(90.f));
	math::set_rotation_x(view_rotations_[2], math::degrees_to_radians(90.f));
	math::set_rotation_x(view_rotations_[3], math::degrees_to_radians(-90.f));
	view_rotations_[4] = math::float3x3::identity;
	math::set_rotation_y(view_rotations_[5], math::degrees_to_radians(180.f));
}

uint32_t Cubic::num_views() const {
	return 6;
}

math::uint2 Cubic::sensor_dimensions() const {
	return sensor_dimensions_;
}

math::uint2 Cubic::sensor_pixel(math::uint2 pixel, uint32_t view) const {
	return view_offsets_[view] + pixel;
}

void Cubic::update_focus(rendering::Worker& /*worker*/) {}

void Cubic::generate_ray(const sampler::Camera_sample& sample, uint32_t view, math::Oray& ray) const {
	math::float2 coordinates =  math::float2(sample.pixel) + sample.pixel_uv;

	math::float3 direction = left_top_ + coordinates.x * d_x_ + coordinates.y * d_y_;

	direction *= view_rotations_[view];

	math::Ray<float> r(math::float3::identity, direction);

	entity::Composed_transformation transformation;
	transformation_at(ray.time, transformation);
	ray.origin = math::transform_point(transformation.object_to_world, r.origin);
	ray.set_direction(math::transform_vector(transformation.object_to_world, math::normalized(r.direction)));
	ray.min_t = 0.f;
	ray.max_t = ray_max_t_;
	ray.depth = 0;
}

}}
