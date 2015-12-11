#include "camera_perspective.hpp"
#include "rendering/sensor/sensor.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/prop/prop_intersection.hpp"
#include "sampler/camera_sample.hpp"
#include "base/math/math.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/ray.inl"
#include "base/math/sampling/sampling.inl"

namespace scene { namespace camera {

Perspective::Perspective(math::uint2 resolution, float ray_max_t, float frame_duration, bool motion_blur,
						 const Focus& focus, float fov, float lens_radius) :
	Camera(resolution, ray_max_t, frame_duration, motion_blur), focus_(focus),
	lens_radius_(lens_radius), focal_distance_(focus_.distance) {
	math::float2 fr(resolution);
	float ratio = fr.x / fr.y;

	float z = ratio * math::Pi / fov * 0.5f;

	left_top_ = math::float3(-ratio,  1.f, z);
	math::float3 right_top	( ratio,  1.f, z);
	math::float3 left_bottom(-ratio, -1.f, z);

	d_x_ = (right_top - left_top_)   / fr.x;
	d_y_ = (left_bottom - left_top_) / fr.y;

	focus_.point.x *= fr.x;
	focus_.point.y *= fr.y;
}

uint32_t Perspective::num_views() const {
	return 1;
}

math::uint2 Perspective::sensor_dimensions() const {
	return resolution_;
}

math::uint2 Perspective::sensor_pixel(math::uint2 pixel, uint32_t /*view*/) const {
	return pixel;
}

void Perspective::update_focus(rendering::Worker& worker) {
	if (focus_.use_point) {
		math::float3 direction = left_top_ + focus_.point.x * d_x_ + focus_.point.y * d_y_;

		entity::Composed_transformation transformation;
		transformation_at(0.f, transformation);

		math::Oray ray;
		ray.origin = transformation.position;
		ray.set_direction(math::transform_vector(transformation.object_to_world, math::normalized(direction)));
		ray.min_t = 0.f;
		ray.max_t = ray_max_t_;
		ray.depth = 0;

		Intersection intersection;
		if (worker.intersect(ray, intersection)) {
			focal_distance_ = ray.max_t + focus_.point.z;
		} else {
			focal_distance_ = focus_.distance;
		}
	}
}

void Perspective::generate_ray(const sampler::Camera_sample& sample, uint32_t /*view*/, math::Oray& ray) const {
	math::float2 coordinates =  math::float2(sample.pixel) + sample.pixel_uv;

	math::float3 direction = left_top_ + coordinates.x * d_x_ + coordinates.y * d_y_;

	math::Ray<float> r(math::float3::identity, direction);

	if (lens_radius_ > 0.f) {
		math::float2 lens  = math::sample_disk_concentric(sample.lens_uv);
		math::float3 focus = r.point(focal_distance_ / r.direction.z);

		r.origin = math::float3(lens.x * lens_radius_, lens.y * lens_radius_, 0.f);
		r.direction = focus - r.origin;
	}

	entity::Composed_transformation transformation;
	transformation_at(ray.time, transformation);
	ray.origin = math::transform_point(transformation.object_to_world, r.origin);
	ray.set_direction(math::transform_vector(transformation.object_to_world, math::normalized(r.direction)));
	ray.min_t = 0.f;
	ray.max_t = ray_max_t_;
	ray.depth = 0;
}

}}
