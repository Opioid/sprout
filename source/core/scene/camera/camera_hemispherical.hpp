#pragma once

#include "camera.hpp"

namespace scene { namespace camera {

class Hemispherical : public Camera {

public:

	Hemispherical(int2 resolution);

	virtual uint32_t num_views() const final override;

	virtual int2 sensor_dimensions() const final override;

	virtual math::Recti view_bounds(uint32_t view) const final override;

	virtual float pixel_solid_angle() const final override;

	virtual void update(rendering::Worker& worker) final override;

	virtual bool generate_ray(const sampler::Camera_sample& sample, uint32_t view,
							  scene::Ray& ray) const final override;

private:

	virtual void set_parameter(const std::string& name, const json::Value& value) final override;

	float d_x_;
	float d_y_;
};

}}
