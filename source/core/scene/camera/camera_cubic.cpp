#include "camera_cubic.hpp"
#include "scene/scene_ray.inl"
#include "rendering/sensor/sensor.hpp"
#include "sampler/camera_sample.hpp"
#include "base/math/math.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/ray.inl"
#include "base/math/sampling/sampling.inl"

namespace scene { namespace camera {

Cubic::Cubic(Layout layout, int2 resolution, float ray_max_t) :
	Camera(int2(resolution.x, resolution.x), ray_max_t) {
	float f = static_cast<float>(resolution.x);

	left_top_ = float3(-1.f, 1.f, 1.f);

	float3 right_top  ( 1.f,  1.f, 1.f);
	float3 left_bottom(-1.f, -1.f, 1.f);

	d_x_ = (right_top - left_top_)   / f;
	d_y_ = (left_bottom - left_top_) / f;

	if (Layout::xmxymyzmz == layout) {
		for (uint32_t i = 0; i < 6; ++i) {
			int2 offset = int2(resolution.x * i, 0);

			view_bounds_[i] = math::Recti{offset, offset + resolution_};
		}

		sensor_dimensions_ = int2(resolution_.x * 6, resolution_.x);
	} else if (Layout::xmxy_myzmz == layout) {
		int2 offset = int2(resolution.x * 0, 0);
		view_bounds_[0] = math::Recti{offset, offset + resolution_};

		offset = int2(resolution.x * 1, 0);
		view_bounds_[1] = math::Recti{offset, offset + resolution_};

		offset = int2(resolution.x * 2, 0);
		view_bounds_[2] = math::Recti{offset, offset + resolution_};

		offset = int2(resolution.x * 0, resolution.x);
		view_bounds_[3] = math::Recti{offset, offset + resolution_};

		offset = int2(resolution.x * 1, resolution.x);
		view_bounds_[4] = math::Recti{offset, offset + resolution_};

		offset = int2(resolution.x * 2, resolution.x);
		view_bounds_[5] = math::Recti{offset, offset + resolution_};

		sensor_dimensions_ = int2(resolution_.x * 3, resolution_.x * 2);
	}

	math::set_rotation_y(view_rotations_[0], math::degrees_to_radians(-90.f));
	math::set_rotation_y(view_rotations_[1], math::degrees_to_radians( 90.f));
	math::set_rotation_x(view_rotations_[2], math::degrees_to_radians( 90.f));
	math::set_rotation_x(view_rotations_[3], math::degrees_to_radians(-90.f));
	view_rotations_[4] = math::float3x3::identity;
	math::set_rotation_y(view_rotations_[5], math::degrees_to_radians(180.f));
}

uint32_t Cubic::num_views() const {
	return 6;
}

int2 Cubic::sensor_dimensions() const {
	return sensor_dimensions_;
}

math::Recti Cubic::view_bounds(uint32_t view) const {
	return view_bounds_[view];
}

void Cubic::update_focus(rendering::Worker& /*worker*/) {}

bool Cubic::generate_ray(const sampler::Camera_sample& sample, uint32_t view,
						 scene::Ray& ray) const {
	float2 coordinates = float2(sample.pixel) + sample.pixel_uv;

	float3 direction = left_top_ + coordinates.x * d_x_ + coordinates.y * d_y_;

	direction = math::normalized(direction * view_rotations_[view]);

	entity::Composed_transformation temp;
	auto& transformation = transformation_at(0.f, temp);

	ray.origin = math::transform_point(math::float3_identity, transformation.object_to_world);
	ray.set_direction(math::transform_vector(direction, transformation.object_to_world));
	ray.min_t = 0.f;
	ray.max_t = ray_max_t_;
	ray.time  = sample.time;
	ray.depth = 0;

	return true;
}

void Cubic::set_parameter(const std::string& /*name*/,
						  const json::Value& /*value*/) {}

}}
