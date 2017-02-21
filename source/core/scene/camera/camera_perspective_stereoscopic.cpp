#include "camera_perspective_stereoscopic.hpp"
#include "rendering/sensor/sensor.hpp"
#include "sampler/camera_sample.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "base/json/json.hpp"
#include "base/math/math.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/ray.inl"
#include "base/math/sampling/sampling.inl"

namespace scene { namespace camera {

Perspective_stereoscopic::Perspective_stereoscopic(int2 resolution) :
	Stereoscopic(resolution) {
	set_fov(90.f);

	view_bounds_[0] = int4{int2(0, 0), resolution};
	view_bounds_[1] = int4{int2(resolution.x, 0),
								  int2(resolution.x * 2, resolution.y)};
}

uint32_t Perspective_stereoscopic::num_views() const {
	return 2;
}

int2 Perspective_stereoscopic::sensor_dimensions() const {
	return int2(resolution_.x * 2, resolution_.y);
}

int4 Perspective_stereoscopic::view_bounds(uint32_t view) const {
	return view_bounds_[view];
}

float Perspective_stereoscopic::pixel_solid_angle() const {
	return 1.f;
}

void Perspective_stereoscopic::update(rendering::Worker& /*worker*/) {}

bool Perspective_stereoscopic::generate_ray(const sampler::Camera_sample& sample,
											uint32_t view, scene::Ray& ray) const {
	float2 coordinates =  float2(sample.pixel) + sample.pixel_uv;

	float3 direction = left_top_ + coordinates.x * d_x_ + coordinates.y * d_y_;
	direction = math::normalized(direction);

	entity::Composed_transformation temp;
	auto& transformation = transformation_at(sample.time, temp);

	ray.origin = math::transform_point(eye_offsets_[view], transformation.object_to_world);
	ray.set_direction(math::transform_vector(direction, transformation.object_to_world));
	ray.min_t = 0.f;
	ray.max_t = Ray_max_t;
	ray.time  = sample.time;
	ray.depth = 0;

	return true;
}

void Perspective_stereoscopic::set_fov(float fov) {
	float2 fr(resolution_);
	float ratio = fr.x / fr.y;

	float z = ratio * math::Pi / fov * 0.5f;

	left_top_ = float3(-ratio,  1.f, z);
	float3 right_top	( ratio,  1.f, z);
	float3 left_bottom(-ratio, -1.f, z);

	d_x_ = (right_top   - left_top_) / fr.x;
	d_y_ = (left_bottom - left_top_) / fr.y;
}

void Perspective_stereoscopic::set_parameter(const std::string& name,
											 const json::Value& value) {
	if ("fov" == name) {
		set_fov(math::degrees_to_radians(json::read_float(value)));
	} else if ("stereo" == name) {
		set_interpupillary_distance(json::read_float(value, "ipd"));
	}
}

}}
