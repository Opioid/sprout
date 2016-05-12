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

	Perspective(math::int2 resolution, float ray_max_t,
				float fov, float lens_radius, const Focus& focus);

	virtual void set_parameters(const json::Value& parameters) final override;

	virtual uint32_t num_views() const final override;

	virtual math::int2 sensor_dimensions() const final override;

	virtual math::Recti view_bounds(uint32_t view) const final override;

	virtual void update_focus(rendering::Worker& worker) final override;

	virtual bool generate_ray(const sampler::Camera_sample& sample, uint32_t view,
							  scene::Ray& ray) const final override;

	void set_fov(float fov);

	void set_lens_radius(float lens_radius);

	void set_focus(const Focus& focus);


protected:

	static void load_focus(const json::Value& focus_value, Focus& focus);

private:

	Focus focus_;
	float lens_radius_;

	math::float3 left_top_;
	math::float3 d_x_;
	math::float3 d_y_;

	float focal_distance_;
};

}}
