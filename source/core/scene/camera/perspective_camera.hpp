#pragma once

#include "camera.hpp"

namespace scene { namespace camera {

class Perspective : public Camera {
public:

	Perspective(const math::float2& dimensions, rendering::film::Film* film, float fov, float lens_radius, float focal_distance);

	virtual void update_view();

	virtual void generate_ray(const rendering::sampler::Camera_sample& sample, math::Oray& ray) const;

private:

	float fov_;
	float lens_radius_;
	float focal_distance_;

	math::float3 left_top_;
	math::float3 d_x_, d_y_;
};

}}
