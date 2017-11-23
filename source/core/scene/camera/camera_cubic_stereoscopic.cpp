#include "camera_cubic_stereoscopic.hpp"
#include "rendering/sensor/sensor.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "sampler/camera_sample.hpp"
#include "base/json/json.hpp"
#include "base/math/math.hpp"
#include "base/math/vector4.inl"
#include "base/math/matrix4x4.inl"
#include "base/math/sampling/sampling.hpp"

namespace scene::camera {

Cubic_stereoscopic::Cubic_stereoscopic(Layout layout, int2 resolution) :
	Stereoscopic(int2(resolution[0], resolution[0])) {
	const float f = static_cast<float>(resolution[0]);

	left_top_ = float3(-1.f, 1.f, 1.f);

	const float3 right_top  ( 1.f,  1.f, 1.f);
	const float3 left_bottom(-1.f, -1.f, 1.f);

	d_x_ = (right_top - left_top_)   / f;
	d_y_ = (left_bottom - left_top_) / f;

	if (Layout::lxlmxlylmylzlmzrxrmxryrmyrzrmz == layout) {
		for (uint32_t i = 0; i < 12; ++i) {
			int2 offset = int2(resolution[0] * i, 0);

			view_bounds_[i] = int4(offset, offset + resolution_);
		}

		sensor_dimensions_ = int2(resolution_[0] * 12, resolution_[0]);
	} else if (Layout::rxlmxryrmyrzrmzlxlmxlylmylzlmz == layout) {
		for (uint32_t i = 0; i < 6; ++i) {
			int2 offset = int2(resolution[0] * (i + 6), 0);

			view_bounds_[i] = int4(offset, offset + resolution_ - int2(1));
		}

		for (uint32_t i = 6; i < 12; ++i) {
			int2 offset = int2(resolution[0] * (i - 6), 0);

			view_bounds_[i] = int4(offset, offset + resolution_ - int2(1));
		}

		sensor_dimensions_ = int2(resolution_[0] * 12, resolution_[0]);
	}

	math::set_rotation_y(view_rotations_[0], math::degrees_to_radians(-90.f));
	math::set_rotation_y(view_rotations_[1], math::degrees_to_radians( 90.f));
	math::set_rotation_x(view_rotations_[2], math::degrees_to_radians( 90.f));
	math::set_rotation_x(view_rotations_[3], math::degrees_to_radians(-90.f));
	view_rotations_[4] = float3x3::identity();
	math::set_rotation_y(view_rotations_[5], math::degrees_to_radians(180.f));

	set_interpupillary_distance_falloff(0.36f);
}

uint32_t Cubic_stereoscopic::num_views() const {
	return 12;
}

int2 Cubic_stereoscopic::sensor_dimensions() const {
	return sensor_dimensions_;
}

int4 Cubic_stereoscopic::view_bounds(uint32_t view) const {
	return view_bounds_[view];
}

float Cubic_stereoscopic::pixel_solid_angle() const {
	return 1.f;
}

bool Cubic_stereoscopic::generate_ray(const sampler::Camera_sample& sample,
									  uint32_t view, scene::Ray& ray) const {
	const float2 coordinates = float2(sample.pixel) + sample.pixel_uv;

	float3 direction = left_top_ + coordinates[0] * d_x_ + coordinates[1] * d_y_;

	const uint32_t face = view % 6;
	direction = math::normalize(direction * view_rotations_[face]);

	const float a = -std::atan2(direction[0], direction[2]);

	float3x3 rotation;
	math::set_rotation_y(rotation, a);

	const float ipd_scale = 1.f - std::pow(std::abs(direction[1]), 12.f - ipd_falloff_ * 10.f);

	const uint32_t eye = view < 6 ? 0 : 1;
	const float3 eye_offset = (ipd_scale * eye_offsets_[eye]) * rotation;

	entity::Composed_transformation temp;
	const auto& transformation = transformation_at(sample.time, temp);

	ray.origin = math::transform_point(eye_offset, transformation.object_to_world);
	ray.set_direction(math::transform_vector(direction, transformation.object_to_world));
	ray.min_t = 0.f;
	ray.max_t = Ray_max_t;
	ray.time  = sample.time;
	ray.depth = 0;

	return true;
}

void Cubic_stereoscopic::set_interpupillary_distance_falloff(float ipd_falloff) {
	ipd_falloff_ = std::sqrt(ipd_falloff);
}

void Cubic_stereoscopic::on_update(rendering::Worker& /*worker*/) {}

void Cubic_stereoscopic::set_parameter(const std::string& name,
									   const json::Value& value) {
	if ("stereo" == name) {
		for (auto n = value.MemberBegin(); n != value.MemberEnd(); ++n) {
			const std::string node_name = n->name.GetString();
			const json::Value& node_value = n->value;

			if ("ipd" == node_name) {
				set_interpupillary_distance(json::read_float(node_value));
			} else if ("ipd_falloff" == node_name) {
				set_interpupillary_distance_falloff(json::read_float(node_value));
			}
		}
	}
}

}
