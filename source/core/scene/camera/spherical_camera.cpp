#include "spherical_camera.hpp"
#include "rendering/film/film.hpp"
#include "sampler/camera_sample.hpp"
#include "base/math/math.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/ray.inl"
#include "base/math/sampling/sampling.inl"

namespace scene { namespace camera {

Spherical::Spherical(math::float2 dimensions, rendering::film::Film* film, float ray_max_t,
					 float frame_duration, bool motion_blur) :
	Camera(dimensions, film, ray_max_t, frame_duration, motion_blur) {
	d_x_ = 1.f / static_cast<float>(film_->dimensions().x);
	d_y_ = 1.f / static_cast<float>(film_->dimensions().y);
}

void Spherical::update_focus(rendering::Worker& /*worker*/) {}

void Spherical::generate_ray(const sampler::Camera_sample& sample, math::Oray& ray) const {
	float x = d_x_ * sample.coordinates.x;
	float y = d_y_ * sample.coordinates.y;

	float phi   = (-x + 0.25f) * 2.f * math::Pi;
	float theta = y * math::Pi;

	float sin_theta = std::sin(theta);
	float cos_theta = std::cos(theta);
	float sin_phi   = std::sin(phi);
	float cos_phi   = std::cos(phi);

	math::float3 dir(sin_theta * cos_phi, cos_theta, sin_theta * sin_phi);

	entity::Composed_transformation transformation;
	transformation_at(ray.time, transformation);
	ray.origin = transformation.position;
//	ray.set_direction(math::transform_vector(transformation.rotation, dir));
	ray.set_direction(dir);
	ray.min_t = 0.f;
	ray.max_t = ray_max_t_;
	ray.depth = 0;
}

}}
