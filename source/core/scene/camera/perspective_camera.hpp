#pragma once

#include "camera.hpp"

namespace scene { namespace camera {

class Perspective : public Camera {
public:

	struct Focus {
		math::float3 point;
		bool use_point = false;

		float distance;
	};

	Perspective(const math::float2& dimensions, rendering::film::Film* film,
				float frame_duration, bool motion_blur, const Focus& focus,
				float fov, float lens_radius, float ray_max_t);

	virtual void update_focus(rendering::Worker& worker) final override;

	virtual void generate_ray(const sampler::Camera_sample& sample,
							  float normalized_tick_offset, float normalized_tick_slice,
							  math::Oray& ray) const final override;

private:

	Focus focus_;
	float fov_;
	float lens_radius_;
	float ray_max_t_;

	math::float3 left_top_;
	math::float3 d_x_, d_y_;

	float focal_distance_;
};

}}
