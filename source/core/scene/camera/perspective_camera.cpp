#include "perspective_camera.hpp"
#include "rendering/film/film.hpp"
#include "rendering/sampler/camera_sample.hpp"
#include "base/math/math.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/ray.inl"

namespace scene { namespace camera {

Perspective::Perspective(const math::float2& dimensions, rendering::film::Film* film, float fov) :
	Camera(dimensions, film), fov_(fov) {}

void Perspective::update_view() {
	float ratio = dimensions_.x / dimensions_.y;

	float z = ratio * math::pi / fov_ * 0.5f;

	left_top_ = math::float3(-ratio,  1.f, z);
	math::float3 right_top	( ratio,  1.f, z);
	math::float3 left_bottom(-ratio, -1.f, z);

	d_x_ = (right_top - left_top_)   / static_cast<float>(film_->dimensions().x);
	d_y_ = (left_bottom - left_top_) / static_cast<float>(film_->dimensions().y);
}

void Perspective::generate_ray(const rendering::sampler::Camera_sample& sample, math::Oray& ray) const {
	/*
direction := p.leftTop.Add(p.dx.Scale(sample.Coordinates.X)).Add(p.dy.Scale(sample.Coordinates.Y))

r := math.Ray{math.MakeVector3(0.0, 0.0, 0.0), direction, 0.0, 1000.0}

if p.lensRadius > 0.0 {
	lensU, lensV := math.SampleDiskConcentric(sample.LensUv.X, sample.LensUv.Y)
	lensUv := math.MakeVector3(lensU * p.lensRadius, lensV * p.lensRadius, 0.0)

	ft := p.focalDistance / r.Direction.Z
	focus := r.Point(ft)

	r.Origin = lensUv
	r.Direction = focus.Sub(r.Origin)
}

ray.Time = math32.Lerp(shutterOpen, shutterClose, sample.Time)

p.entity.TransformationAt(ray.Time, transformation)

ray.Origin = transformation.ObjectToWorld.TransformPoint(r.Origin)
ray.SetDirection(transformation.ObjectToWorld.TransformVector3(r.Direction.Normalized()))
ray.MaxT  = 1000.0
ray.Depth = 0*/


	math::float3 direction = left_top_ + sample.coordinates.x * d_x_ + sample.coordinates.y * d_y_;

	math::Ray<float> r(math::float3::identity, direction);

	ray.time = 0.f;

	scene::Composed_transformation transformation;
	transformation_at(ray.time, transformation);
	ray.origin = math::transform_point(transformation.object_to_world, r.origin);
	ray.set_direction(math::transform_vector(transformation.object_to_world, normalized(r.direction)));
	ray.min_t = 0.f;
	ray.max_t = 1000.f;
	ray.depth = 0;
}

}}
