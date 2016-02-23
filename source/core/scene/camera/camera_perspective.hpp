#pragma once

#include "camera.hpp"

namespace scene { namespace camera {

class Perspective : public Camera {
public:

	struct Focus {
		math::vec3 point = math::vec3(0.5f, 0.5f, 0.f);
		bool use_point = true;

		float distance;
	};

	Perspective(math::int2 resolution, float ray_max_t, float frame_duration, bool motion_blur,
				const Focus& focus, float fov, float lens_radius);

	virtual uint32_t num_views() const final override;

	virtual math::int2 sensor_dimensions() const final override;

	virtual math::Recti view_bounds(uint32_t view) const final override;

	virtual void update_focus(rendering::Worker& worker) final override;

	virtual void generate_ray(const sampler::Camera_sample& sample, uint32_t view,
							  scene::Ray& ray) const final override;

private:

	Focus focus_;
	float lens_radius_;

	math::vec3 left_top_;
	math::vec3 d_x_;
	math::vec3 d_y_;

	float focal_distance_;
};

}}
