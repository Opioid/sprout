#include "perspective_camera.hpp"
#include "rendering/film/film.hpp"
#include "sampler/camera_sample.hpp"
#include "base/math/math.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/ray.inl"
#include "base/math/sampling.hpp"

namespace scene { namespace camera {

Perspective::Perspective(const math::float2& dimensions, rendering::film::Film* film, float fov, float lens_radius, float focal_distance) :
	Camera(dimensions, film), fov_(fov), lens_radius_(lens_radius), focal_distance_(focal_distance) {}

void Perspective::update_view() {
	float ratio = dimensions_.x / dimensions_.y;

	float z = ratio * math::Pi / fov_ * 0.5f;

	left_top_ = math::float3(-ratio,  1.f, z);
	math::float3 right_top	( ratio,  1.f, z);
	math::float3 left_bottom(-ratio, -1.f, z);

	d_x_ = (right_top - left_top_)   / static_cast<float>(film_->dimensions().x);
	d_y_ = (left_bottom - left_top_) / static_cast<float>(film_->dimensions().y);
}

void Perspective::generate_ray(const sampler::Camera_sample& sample, math::Oray& ray) const {
	math::float3 direction = left_top_ + sample.coordinates.x * d_x_ + sample.coordinates.y * d_y_;

	math::Ray<float> r(math::float3::identity, direction);

	if (lens_radius_ > 0.f) {
		math::float2 lens  = math::sample_disk_concentric(sample.lens_uv);
		math::float3 focus = r.point(focal_distance_ / r.direction.z);

		r.origin = math::float3(lens.x * lens_radius_, lens.y * lens_radius_, 0.f);
		r.direction = focus - r.origin;
	}

	ray.time = 0.f;

	entity::Composed_transformation transformation;
	transformation_at(ray.time, transformation);
	ray.origin = math::transform_point(transformation.object_to_world, r.origin);
	ray.set_direction(math::transform_vector(transformation.object_to_world, math::normalized(r.direction)));
	ray.min_t = 0.f;
	ray.max_t = 1000.f;
	ray.depth = 0;
}

}}
