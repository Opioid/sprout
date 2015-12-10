#pragma once

#include "camera.hpp"

namespace scene { namespace camera {

class Spherical : public Camera {
public:

	Spherical(math::uint2 resolution, float ray_max_t, float frame_duration, bool motion_blur);

	virtual uint32_t num_views() const final override;

	virtual math::uint2 sensor_dimensions() const final override;

	virtual math::uint2 sensor_pixel(math::uint2 pixel, uint32_t view) const final override;

	virtual void update_focus(rendering::Worker& worker) final override;

	virtual void generate_ray(const sampler::Camera_sample& sample, uint32_t view,
							  math::Oray& ray) const final override;

private:

	float d_x_;
	float d_y_;
};

}}
