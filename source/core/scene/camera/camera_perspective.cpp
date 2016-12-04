#include "camera_perspective.hpp"
#include "rendering/sensor/sensor.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/scene_ray.inl"
#include "scene/scene_intersection.hpp"
#include "sampler/camera_sample.hpp"
#include "base/json/json.hpp"
#include "base/math/math.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/sampling/sampling.inl"

#include <iostream>

namespace scene { namespace camera {

Perspective::Perspective(int2 resolution, float ray_max_t) :
	Camera(resolution, ray_max_t),
	lens_tilt_(float3x3::identity()),
	lens_shift_(0.f, 0.f),
	lens_radius_(0.f),
	fov_(math::degrees_to_radians(60.f)) {}

uint32_t Perspective::num_views() const {
	return 1;
}

int2 Perspective::sensor_dimensions() const {
	return resolution_;
}

math::Recti Perspective::view_bounds(uint32_t /*view*/) const {
	return math::Recti{int2(0, 0), resolution_};
}

float Perspective::pixel_solid_angle() const {
	return fov_ / static_cast<float>(resolution_.x);
}

void Perspective::update(rendering::Worker& worker) {
	float2 fr(resolution_);
	float ratio = fr.x / fr.y;

	float z = ratio * math::Pi / fov_ * 0.5f;

//	float3 left_top   (-ratio,  1.f, z);
//	float3 right_top  ( ratio,  1.f, z);
//	float3 left_bottom(-ratio, -1.f, z);

	float3 left_top    = float3(-ratio,  1.f, 0.f) * lens_tilt_;
	float3 right_top   = float3( ratio,  1.f, 0.f) * lens_tilt_;
	float3 left_bottom = float3(-ratio, -1.f, 0.f) * lens_tilt_;

	left_top.z += z;
	right_top.z += z;
	left_bottom.z += z;

	z_ = z;

	left_top_ = left_top + float3(lens_shift_, 0.f);
	d_x_ = (right_top   - left_top) / fr.x;
	d_y_ = (left_bottom - left_top) / fr.y;

	update_focus(worker);

	float3 gleft_top    = 500.f * float3(-ratio,  1.f, 0.f) * lens_tilt_;
	float3 gright_top   = 500.f * float3( ratio,  1.f, 0.f) * lens_tilt_;
	float3 gleft_bottom = 500.f * float3(-ratio, -1.f, 0.f) * lens_tilt_;

	gleft_top.z += focus_distance_;
	gright_top.z += focus_distance_;
	gleft_bottom.z += focus_distance_;

	gleft_top_ = gleft_top;// + float3(lens_shift_, 0.f);
	gd_x_ = (gright_top   - gleft_top) / fr.x;
	gd_y_ = (gleft_bottom - gleft_top) / fr.y;
}

bool Perspective::generate_ray(const sampler::Camera_sample& sample,
							   uint32_t /*view*/, scene::Ray& ray) const {
	float2 coordinates = float2(sample.pixel) + sample.pixel_uv;

	float3 direction = left_top_ + coordinates.x * d_x_ + coordinates.y * d_y_;

//	direction.z += z_;

	float3 origin;

	if (lens_radius_ > 0.f) {
		float2 lens = math::sample_disk_concentric(sample.lens_uv);

		float3 thing = gleft_top_ + coordinates.x * gd_x_ + coordinates.y * gd_y_;
	//	float3 thing = float3(0.f, 0.f, focus_distance_) * lens_tilt_;

		float t = focus_distance_ / /*direction.z;//*/z_;


		float degdeg = std::max(thing.z, 0.125f) / z_;
		t = degdeg;
	//	std::cout << t << " : " << degdeg << std::endl;

	//	float t = thing.z / direction.z;//z_;
		float3 focus = t * direction;

		origin = float3(lens_radius_ * lens, 0.f);
		direction = focus - origin;
	} else {
		origin = math::float3_identity;
	}

	entity::Composed_transformation temp;
	auto& transformation = transformation_at(sample.time, temp);

	float3 origin_w = math::transform_point(origin, transformation.object_to_world);

	direction = math::normalized(direction);
	float3 direction_w = math::transform_vector(direction, transformation.object_to_world);

	ray.origin = origin_w;
	ray.set_direction(direction_w);
	ray.min_t = 0.f;
	ray.max_t = ray_max_t_;
	ray.time  = sample.time;
	ray.depth = 0;

	return true;
}

void Perspective::set_fov(float fov) {
	fov_ = fov;
}

void Perspective::set_lens(const Lens& lens) {
	float a = math::degrees_to_radians(lens.angle);
	float c = std::cos(a);
	float s = std::sin(a);

	float3 axis(c, s, 0.f);
	float tilt = math::degrees_to_radians(lens.tilt);
	math::set_rotation(lens_tilt_, axis, tilt);

	float shift = 2.f * lens.shift;

	lens_shift_  = float2(-s * shift, c * shift);
	lens_radius_ = lens.radius;
}

void Perspective::set_focus(const Focus& focus) {
	focus_ = focus;

	focus_.point.xy *= float2(resolution_);

	focus_distance_ = focus_.distance;
}

void Perspective::update_focus(rendering::Worker& worker) {
	if (focus_.use_point) {
		float3 direction = left_top_ + focus_.point.x * d_x_ + focus_.point.y * d_y_;
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
			focus_distance_ = ray.max_t + focus_.point.z;
		} else {
			focus_distance_ = focus_.distance;
		}
	}
}

void Perspective::set_parameter(const std::string& name, const json::Value& value) {
	if ("fov" == name) {
		set_fov(math::degrees_to_radians(json::read_float(value)));
	} else if ("lens" == name) {
		Lens lens;
		load_lens(value, lens);
		set_lens(lens);
	} else if ("focus" == name) {
		Focus focus;
		load_focus(value, focus);
		set_focus(focus);
	}
}

void Perspective::load_lens(const json::Value& lens_value, Lens& lens) {
	for (auto& n : lens_value.GetObject()) {
		if ("angle" == n.name) {
			lens.angle = json::read_float(n.value);
		} else if ("shift" == n.name) {
			lens.shift = json::read_float(n.value);
		} else if ("tilt" == n.name) {
			lens.tilt = json::read_float(n.value);
		} else if ("radius" == n.name) {
			lens.radius = json::read_float(n.value);
		}
	}
}

void Perspective::load_focus(const json::Value& focus_value, Focus& focus) {
	focus.use_point = false;

	for (auto& n : focus_value.GetObject()) {
		if ("point" == n.name) {
			focus.point = json::read_float3(n.value);
			focus.use_point = true;
		} else if ("distance" == n.name) {
			focus.distance = json::read_float(n.value);
		}
	}
}

}}
