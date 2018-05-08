#include "camera_perspective.hpp"
#include "rendering/sensor/sensor.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/scene_worker.hpp"
#include "scene/prop/prop_intersection.hpp"
#include "sampler/camera_sample.hpp"
#include "base/json/json.hpp"
#include "base/math/math.hpp"
#include "base/math/vector4.inl"
#include "base/math/matrix4x4.inl"
#include "base/math/plane.inl"
#include "base/math/sampling/sampling.hpp"

namespace scene::camera {

Perspective::Perspective(int2 resolution) :
	Camera(resolution),
	fov_(math::degrees_to_radians(60.f)) {}

uint32_t Perspective::num_views() const {
	return 1;
}

int2 Perspective::sensor_dimensions() const {
	return resolution_;
}

int4 Perspective::view_bounds(uint32_t /*view*/) const {
	return int4(int2(0, 0), resolution_ - int2(1));
}

float Perspective::pixel_solid_angle() const {
	return fov_ / static_cast<float>(resolution_[0]);
}

bool Perspective::generate_ray(sampler::Camera_sample const& sample,
							   uint32_t /*view*/, scene::Ray& ray) const {
	float2 const coordinates = float2(sample.pixel) + sample.pixel_uv;

	float3 direction = left_top_ + coordinates[0] * d_x_ + coordinates[1] * d_y_;

	float3 origin;

	if (lens_radius_ > 0.f) {
		float2 const lens = math::sample_disk_concentric(sample.lens_uv);

		origin = float3(lens_radius_ * lens, 0.f);

		float const t = focus_distance_ / direction[2];
		float3 const focus = t * direction;

		direction = focus - origin;
	} else {
		origin = float3::identity();
	}

	entity::Composed_transformation temp;
	auto const& transformation = transformation_at(sample.time, temp);

	float3 const origin_w = math::transform_point(origin, transformation.object_to_world);

	direction = math::normalize(direction);
	float3 const direction_w = math::transform_vector(direction, transformation.object_to_world);

	ray = create_ray(origin_w, direction_w, sample.time);

	return true;
}

void Perspective::set_fov(float fov) {
	fov_ = fov;
}

void Perspective::set_lens(const Lens& lens) {
	float const a = math::degrees_to_radians(lens.angle);
	float const c = std::cos(a);
	float const s = std::sin(a);

	float3 const axis(c, s, 0.f);
	float const tilt = math::degrees_to_radians(lens.tilt);
	math::set_rotation(lens_tilt_, axis, tilt);

	float const shift = 2.f * lens.shift;

	lens_shift_  = float2(-s * shift, c * shift);
	lens_radius_ = lens.radius;
}

void Perspective::set_focus(const Focus& focus) {
	focus_ = focus;

	focus_.point[0] *= static_cast<float>(resolution_[0]);
	focus_.point[1] *= static_cast<float>(resolution_[1]);

	focus_distance_ = focus_.distance;
}

void Perspective::on_update(Worker& worker) {
	float2 const fr(resolution_);
	float const ratio = fr[0] / fr[1];

	float const z = ratio * math::Pi / fov_ * 0.5f;

//	float3 left_top   (-ratio,  1.f, z);
//	float3 right_top  ( ratio,  1.f, z);
//	float3 left_bottom(-ratio, -1.f, z);

	float3 left_top    = float3(-ratio,  1.f, 0.f) * lens_tilt_;
	float3 right_top   = float3( ratio,  1.f, 0.f) * lens_tilt_;
	float3 left_bottom = float3(-ratio, -1.f, 0.f) * lens_tilt_;

	left_top[2]    += z;
	right_top[2]   += z;
	left_bottom[2] += z;

	left_top_ = left_top + float3(lens_shift_, 0.f);
	d_x_ = (right_top   - left_top) / fr[0];
	d_y_ = (left_bottom - left_top) / fr[1];

	update_focus(worker);
}

void Perspective::update_focus(Worker& worker) {
	if (focus_.use_point && lens_radius_ > 0.f) {
		float3 direction = left_top_ + focus_.point[0] * d_x_ + focus_.point[1] * d_y_;
		direction = math::normalize(direction);

		entity::Composed_transformation temp;
		auto const& transformation = transformation_at(0.f, temp);

		scene::Ray ray(transformation.position,
					   math::transform_vector(direction, transformation.object_to_world),
					   0.f, Ray_max_t, 0, 0.f);

		prop::Intersection intersection;
		if (worker.intersect(ray, intersection)) {
			focus_distance_ = ray.max_t + focus_.point[2];
		} else {
			focus_distance_ = focus_.distance;
		}
	}
}

void Perspective::set_parameter(std::string_view name, json::Value const& value) {
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

void Perspective::load_lens(json::Value const& lens_value, Lens& lens) {
	for (auto const& n : lens_value.GetObject()) {
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

void Perspective::load_focus(json::Value const& focus_value, Focus& focus) {
	focus.use_point = false;

	for (auto const& n : focus_value.GetObject()) {
		if ("point" == n.name) {
			focus.point = json::read_float3(n.value);
			focus.use_point = true;
		} else if ("distance" == n.name) {
			focus.distance = json::read_float(n.value);
		}
	}
}

}
