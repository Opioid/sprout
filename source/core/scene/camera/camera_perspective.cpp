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
#include "base/math/plane.inl"
#include "base/math/sampling/sampling.inl"

#include <iostream>
#include "base/math/print.hpp"

namespace scene { namespace camera {

bool intersect(float4_p plane, float3_p org, float3_p dir, float3& intersection) {
//	float3_p normal = transformation.rotation.v3.z;
//	float d = math::dot(normal, transformation.position);
	float denom = -math::dot(plane.xyz, dir);
	float numer =  math::dot(plane.xyz, org) + plane.w;
	float hit_t = numer / denom;

	if (hit_t > 0.f) {
		intersection = org + hit_t * dir; // ray.point(hit_t);

		return true;
	}

	return false;
}

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

	float3 left_top    = float3(-ratio,  1.f, 0.f)/*;//*/ * lens_tilt_;
	float3 right_top   = float3( ratio,  1.f, 0.f)/*;//*/ * lens_tilt_;
	float3 left_bottom = float3(-ratio, -1.f, 0.f)/*;//*/ * lens_tilt_;

	left_top.z += z;
	right_top.z += z;
	left_bottom.z += z;

	z_ = z;

	left_top_ = left_top + float3(lens_shift_, 0.f);
	d_x_ = (right_top   - left_top) / fr.x;
	d_y_ = (left_bottom - left_top) / fr.y;

	update_focus(worker);

//return;
	float j = z / std::sin(lens_tilt_a_);

	float4x4 translation;
	math::set_translation(translation, float3(0.f, j, 0.f));

	float w = 0.4f * math::Pi;// 0.f;

	float4x4 rotation;
	math::set_rotation_x(rotation, w);

	float3 durum(0.f, 0.f, 1.f);

	{
		float4x4 trafo = translation * rotation;

	float3 dir = math::transform_vector(durum, trafo);
	float3 org = math::transform_point(float3(0.f, 0.f, 0.f), translation);

	math::plane stuff = math::create_plane(dir, org);


	float3 fleft_top;
	float3 fright_top;
	float3 fleft_bottom;

	if (!intersect(stuff, float3(0.f, 0.f, 0.f), left_top, fleft_top)) {
		fleft_top = left_top;
		fleft_top.z -= z;
	}

	if (!intersect(stuff, float3(0.f, 0.f, 0.f), right_top, fright_top)) {
		fright_top = right_top;
		fright_top.z -= z;
	}

	if (!intersect(stuff, float3(0.f, 0.f, 0.f), left_bottom, fleft_bottom)) {
		fleft_bottom = left_bottom;
		fleft_bottom.z -= z;
	}

	fleft_top_ = fleft_top + float3(lens_shift_, 0.f);
	fd_x_	   = (fright_top   - fleft_top) / fr.x;
	fd_y_	   = (fleft_bottom - fleft_top) / fr.y;

//	std::cout << stuff << std::endl;

	}

	return;

/*
//	float3 fleft_top    = float3(focus_distance_ * -ratio,  focus_distance_, 0.f) * lens_tilt_;
//	float3 fright_top   = float3(focus_distance_ *  ratio,  focus_distance_, 0.f) * lens_tilt_;
//	float3 fleft_bottom = float3(focus_distance_ * -ratio, -focus_distance_, 0.f) * lens_tilt_;

//	fleft_top.z += focus_distance_;
//	fright_top.z += focus_distance_;
//	fleft_bottom.z += focus_distance_;

	float3 fleft_top    = focus_distance_ * math::normalized(left_top_) * lens_tilt_;
	float3 fright_top   = focus_distance_ * math::normalized(right_top) * lens_tilt_;
	float3 fleft_bottom = focus_distance_ * math::normalized(left_bottom) * lens_tilt_;

//	fleft_top.z += focus_distance_;
//	fright_top.z += focus_distance_;
//	fleft_bottom.z += focus_distance_;

	fleft_top_ = fleft_top + float3(lens_shift_, 0.f);
	fd_x_ = (fright_top   - fleft_top) / fr.x;
	fd_y_ = (fleft_bottom - fleft_top) / fr.y;
*/
}

bool Perspective::generate_ray(const sampler::Camera_sample& sample,
							   uint32_t /*view*/, scene::Ray& ray) const {
	float2 coordinates = float2(sample.pixel) + sample.pixel_uv;

	float3 direction = left_top_ + coordinates.x * d_x_ + coordinates.y * d_y_;

//	direction.z += z_;

	float3 origin;

	if (lens_radius_ > 0.f) {
//		float2 lens = math::sample_disk_concentric(sample.lens_uv);

//		origin = float3(lens_radius_ * lens, 0.f);

//		float t = focus_distance_ / direction.z;//z_;
//		float3 focus = t * direction;

//		direction = focus - origin;


		float2 lens = math::sample_disk_concentric(sample.lens_uv);

		origin = float3(lens_radius_ * lens, 0.f);

		float3 fdirection = fleft_top_ + coordinates.x * fd_x_ + coordinates.y * fd_y_;

		float t = (focus_distance_ + fdirection.z) / z_;
		float3 focus = t * direction;

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

	lens_tilt_a_ = tilt;
}

void Perspective::set_focus(const Focus& focus) {
	focus_ = focus;

	focus_.point.xy *= float2(resolution_);

	focus_distance_ = focus_.distance;
}

void Perspective::update_focus(rendering::Worker& worker) {
	if (focus_.use_point && lens_radius_ > 0.f) {
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
