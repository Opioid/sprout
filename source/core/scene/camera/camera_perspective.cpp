#include "camera_perspective.hpp"
#include "rendering/sensor/sensor.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene_ray.inl"
#include "scene/prop/prop_intersection.hpp"
#include "sampler/camera_sample.hpp"
#include "base/json/json.hpp"
#include "base/math/math.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/sampling/sampling.inl"

namespace scene { namespace camera {

Perspective::Perspective(math::int2 resolution, float ray_max_t) :
	Camera(resolution, ray_max_t),
	lens_radius_(0.f) {
	set_fov(60.f);
	set_focus(Focus());
}

uint32_t Perspective::num_views() const {
	return 1;
}

math::int2 Perspective::sensor_dimensions() const {
	return resolution_;
}

math::Recti Perspective::view_bounds(uint32_t /*view*/) const {
	return math::Recti{math::int2(0, 0), resolution_};
}

void Perspective::update_focus(rendering::Worker& worker) {
	if (focus_.use_point) {
		math::float3 direction = left_top_ + focus_.point.x * d_x_ + focus_.point.y * d_y_;
		direction = math::normalized(direction);

		entity::Composed_transformation temp;
		auto& transformation = transformation_at(0.f, temp);

		scene::Ray ray;
		ray.origin = transformation.position;
		ray.set_direction(math::transform_vector(direction, transformation.object_to_world));
		ray.min_t = 0.f;
		ray.max_t = ray_max_t_;
		ray.time = 0.f;
		ray.depth = 0;

		Intersection intersection;
		if (worker.intersect(ray, intersection)) {
			focal_distance_ = ray.max_t + focus_.point.z;
		} else {
			focal_distance_ = focus_.distance;
		}
	}
}

bool Perspective::generate_ray(const sampler::Camera_sample& sample,
							   uint32_t /*view*/,
							   scene::Ray& ray) const {
	math::float2 coordinates = math::float2(sample.pixel) + sample.pixel_uv;

	math::float3 direction = left_top_ + coordinates.x * d_x_ + coordinates.y * d_y_;

	math::float3 origin;

	if (lens_radius_ > 0.f) {
		math::float2 lens = math::sample_disk_concentric(sample.lens_uv);

		float t = focal_distance_ / direction.z;
		math::float3 focus = t * direction;

		origin = math::float3(lens.x * lens_radius_, lens.y * lens_radius_, 0.f);
		direction = focus - origin;
	} else {
		origin = math::float3_identity;
	}

	entity::Composed_transformation temp;
	auto& transformation = transformation_at(sample.time, temp);

	math::float3 origin_w = math::transform_point(origin, transformation.object_to_world);

	direction = math::normalized(direction);
	math::float3 direction_w = math::transform_vector(direction, transformation.object_to_world);

	ray.origin = origin_w;
	ray.set_direction(direction_w);
	ray.min_t = 0.f;
	ray.max_t = ray_max_t_;
	ray.time  = sample.time;
	ray.depth = 0;

	return true;
}

void Perspective::set_fov(float fov) {
	math::float2 fr(resolution_);
	float ratio = fr.x / fr.y;

	float z = ratio * math::Pi / fov * 0.5f;

	left_top_ = math::float3(-ratio,  1.f, z);
	math::float3 right_top	( ratio,  1.f, z);
	math::float3 left_bottom(-ratio, -1.f, z);

	d_x_ = (right_top   - left_top_) / fr.x;
	d_y_ = (left_bottom - left_top_) / fr.y;
}

void Perspective::set_lens_radius(float lens_radius) {
	lens_radius_ = lens_radius;
}

void Perspective::set_focus(const Focus& focus) {
	focus_ = focus;

	math::float2 fr(resolution_);
	focus_.point.x *= fr.x;
	focus_.point.y *= fr.y;

	focal_distance_ = focus_.distance;
}

void Perspective::set_parameter(const std::string& name, const json::Value& value) {
	if ("fov" == name) {
		set_fov(math::degrees_to_radians(json::read_float(value)));
	} else if ("lens_radius" == name) {
		lens_radius_ = json::read_float(value);
	} else if ("focus" == name) {
		Focus focus;
		load_focus(value, focus);
		set_focus(focus);
	}
}

void Perspective::load_focus(const json::Value& focus_value, Focus& focus) {
	focus.use_point = false;

	for (auto n = focus_value.MemberBegin(); n != focus_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("point" == node_name) {
			focus.point = json::read_float3(node_value);
			focus.use_point = true;
		} else if ("distance" == node_name) {
			focus.distance = json::read_float(node_value);
		}
	}
}

}}
