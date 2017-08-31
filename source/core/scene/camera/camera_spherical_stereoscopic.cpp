#include "camera_spherical_stereoscopic.hpp"
#include "rendering/sensor/sensor.hpp"
#include "sampler/camera_sample.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "base/json/json.hpp"
#include "base/math/math.hpp"
#include "base/math/vector4.inl"
#include "base/math/matrix4x4.inl"
#include "base/math/sampling/sampling.hpp"

// the layout should match
// https://developers.google.com/vr/jump/rendering-ods-content.pdf

namespace scene { namespace camera {

Spherical_stereoscopic::Spherical_stereoscopic(int2 resolution) :
	Stereoscopic(resolution) {
	const float2 fr(resolution);
	d_x_ = 1.f / fr[0];
	d_y_ = 1.f / fr[1];

    view_bounds_[0] = int4(int2(0, 0), resolution - int2(1, 1));
	view_bounds_[1] = int4(int2(0, resolution[1]),
						   int2(resolution[0], resolution[1] * 2) - int2(1));
}

uint32_t Spherical_stereoscopic::num_views() const {
	return 2;
}

int2 Spherical_stereoscopic::sensor_dimensions() const {
	return view_bounds_[1].zw();
}

int4 Spherical_stereoscopic::view_bounds(uint32_t view) const {
	return view_bounds_[view];
}

float Spherical_stereoscopic::pixel_solid_angle() const {
	return 1.f;
}

bool Spherical_stereoscopic::generate_ray(const sampler::Camera_sample& sample,
										  uint32_t view, scene::Ray& ray) const {
	const float2 coordinates =  float2(sample.pixel) + sample.pixel_uv;

	const float x = d_x_ * coordinates[0];
	const float y = d_y_ * coordinates[1];

	const float phi   = (x - 0.5f) * (2.f * math::Pi);
	const float theta = y * math::Pi;

	const float sin_phi   = std::sin(phi);
	const float cos_phi   = std::cos(phi);
	const float sin_theta = std::sin(theta);
	const float cos_theta = std::cos(theta);

	const float3 dir(sin_phi * sin_theta, cos_theta, cos_phi * sin_theta);

	float3x3 rotation;
	math::set_rotation_y(rotation, phi);
	const float3 eye_pos = eye_offsets_[view] * rotation;

	entity::Composed_transformation temp;
	const auto& transformation = transformation_at(sample.time, temp);

	ray.origin = math::transform_point(eye_pos, transformation.object_to_world);
	ray.set_direction(math::transform_vector(dir, transformation.rotation));
	ray.min_t = 0.f;
	ray.max_t = Ray_max_t;
	ray.time  = sample.time;
	ray.depth = 0;

	return true;
}

void Spherical_stereoscopic::on_update(rendering::Worker& /*worker*/) {}

void Spherical_stereoscopic::set_parameter(const std::string& name,
										   const json::Value& value) {
	if ("stereo" == name) {
		for (auto n = value.MemberBegin(); n != value.MemberEnd(); ++n) {
			const std::string node_name = n->name.GetString();
			const json::Value& node_value = n->value;

			if ("ipd" == node_name) {
				set_interpupillary_distance(json::read_float(node_value));
			}
		}
	}
}

}}
