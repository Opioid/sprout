#include "camera_spherical_stereoscopic.hpp"
#include "rendering/sensor/sensor.hpp"
#include "sampler/camera_sample.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "base/json/json.hpp"
#include "base/math/math.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/sampling/sampling.inl"

// the layout should match
// https://developers.google.com/vr/jump/rendering-ods-content.pdf

namespace scene { namespace camera {

Spherical_stereoscopic::Spherical_stereoscopic(int2 resolution) :
	Stereoscopic(resolution) {
	float2 fr(resolution);
	d_x_ = 1.f / fr.x;
	d_y_ = 1.f / fr.y;

    view_bounds_[0] = int4(int2(0, 0), resolution - int2(1, 1));
    view_bounds_[1] = int4(int2(0, resolution.y),
                           int2(resolution.x, resolution.y * 2) - int2(1, 1));
}

uint32_t Spherical_stereoscopic::num_views() const {
	return 2;
}

int2 Spherical_stereoscopic::sensor_dimensions() const {
	return view_bounds_[1].v2.zw;
}

int4 Spherical_stereoscopic::view_bounds(uint32_t view) const {
	return view_bounds_[view];
}

float Spherical_stereoscopic::pixel_solid_angle() const {
	return 1.f;
}

void Spherical_stereoscopic::update(rendering::Worker& /*worker*/) {}

bool Spherical_stereoscopic::generate_ray(const sampler::Camera_sample& sample,
										  uint32_t view, scene::Ray& ray) const {
	float2 coordinates =  float2(sample.pixel) + sample.pixel_uv;

	float x = d_x_ * coordinates.x;
	float y = d_y_ * coordinates.y;

	float phi   = (x - 0.5f) * 2.f * math::Pi;
	float theta = y * math::Pi;

	float sin_phi   = std::sin(phi);
	float cos_phi   = std::cos(phi);
	float sin_theta = std::sin(theta);
	float cos_theta = std::cos(theta);

	float3 dir(sin_phi * sin_theta, cos_theta, cos_phi * sin_theta);

	math::float3x3 rotation;
	math::set_rotation_y(rotation, phi);
	float3 eye_pos = eye_offsets_[view] * rotation;

	entity::Composed_transformation temp;
	auto& transformation = transformation_at(sample.time, temp);

	ray.origin = math::transform_point(eye_pos, transformation.object_to_world);
	ray.set_direction(math::transform_vector(dir, transformation.rotation));
	ray.min_t = 0.f;
	ray.max_t = Ray_max_t;
	ray.time  = sample.time;
	ray.depth = 0;

	return true;
}

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
