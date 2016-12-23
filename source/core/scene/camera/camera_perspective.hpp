#pragma once

#include "camera.hpp"

namespace scene { namespace camera {

class Perspective : public Camera {

public:

	struct Lens {
		float angle  = 0.f;
		float shift  = 0.f;
		float tilt   = 0.f;
		float radius = 0.f;
	};

	struct Focus {
		float3 point = float3(0.5f, 0.5f, 0.f);
		float distance;

		bool use_point = false;
	};

	Perspective(int2 resolution);

	virtual uint32_t num_views() const final override;

	virtual int2 sensor_dimensions() const final override;

	virtual math::Recti view_bounds(uint32_t view) const final override;

	virtual float pixel_solid_angle() const final override;

	virtual void update(rendering::Worker& worker) final override;

	virtual bool generate_ray(const sampler::Camera_sample& sample, uint32_t view,
							  scene::Ray& ray) const final override;

	void set_fov(float fov);

	void set_lens(const Lens& lens);

	void set_focus(const Focus& focus);

private:

	void update_focus(rendering::Worker& worker);

	virtual void set_parameter(const std::string& name, const json::Value& value) final override;

	static void load_lens(const json::Value& lens_value, Lens& lens);

	static void load_focus(const json::Value& focus_value, Focus& focus);

	float3 left_top_;
	float3 d_x_;
	float3 d_y_;

	float3x3 lens_tilt_;
	float2	 lens_shift_;
	float	 lens_radius_;

	float fov_;

	Focus focus_;
	float focus_distance_;
};

}}
