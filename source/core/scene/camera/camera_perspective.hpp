#pragma once

#include "camera.hpp"

namespace scene { namespace camera {

class Perspective : public Camera {
public:

	struct Focus {
		math::float3 point = math::float3(0.5f, 0.5f, 0.f);
		bool use_point = true;

		float distance;
	};

	Perspective(math::float2 dimensions, rendering::sensor::Sensor* sensor, float ray_max_t,
				float frame_duration, bool motion_blur, const Focus& focus, float fov, float lens_radius);

	virtual void update_focus(rendering::Worker& worker) final override;

	virtual void generate_ray(const sampler::Camera_sample& sample, math::Oray& ray) const final override;

private:

	Focus focus_;
	float fov_;
	float lens_radius_;

	math::float3 left_top_;
	math::float3 d_x_;
	math::float3 d_y_;

	float focal_distance_;
};

}}
