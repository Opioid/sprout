#pragma once

#include "camera_stereoscopic.hpp"

namespace scene { namespace camera {

class Perspective_stereoscopic : public Stereoscopic {
public:

	Perspective_stereoscopic(float interpupillary_distance,
							 math::int2 resolution, float ray_max_t,
							 float frame_duration, bool motion_blur, float fov);

	virtual uint32_t num_views() const final override;

	virtual math::int2 sensor_dimensions() const final override;

	virtual math::Recti view_bounds(uint32_t view) const final override;

	virtual void update_focus(rendering::Worker& worker) final override;

	virtual void generate_ray(const sampler::Camera_sample& sample, uint32_t view,
							  scene::Ray& ray) const final override;

private:

	float fov_;

	math::vec3 left_top_;
	math::vec3 d_x_;
	math::vec3 d_y_;

	math::Recti view_bounds_[2];
};

}}
