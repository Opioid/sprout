#pragma once

#include "camera.hpp"

namespace scene { namespace camera {

class Spherical : public Camera {
public:

	Spherical(math::float2 dimensions, rendering::film::Film* film, float ray_max_t,
			  float frame_duration, bool motion_blur);

	virtual void update_focus(rendering::Worker& worker) final override;

	virtual void generate_ray(const sampler::Camera_sample& sample, math::Oray& ray) const final override;

private:

	float d_x_;
	float d_y_;
};

}}
