#ifndef SU_CORE_SCENE_CAMERA_SPHERICAL_HPP
#define SU_CORE_SCENE_CAMERA_SPHERICAL_HPP

#include "camera.hpp"

namespace scene::camera {

class Spherical : public Camera {
public:

	Spherical(int2 resolution);

	virtual uint32_t num_views() const override final;

	virtual int2 sensor_dimensions() const override final;

	virtual int4 view_bounds(uint32_t view) const override final;

	virtual float pixel_solid_angle() const override final;

	virtual bool generate_ray(sampler::Camera_sample const& sample, uint32_t view,
							  scene::Ray& ray) const override final;

private:

	virtual void on_update(Worker& worker) override final;

	virtual void set_parameter(std::string_view name, json::Value const& value) override final;

	float d_x_;
	float d_y_;
};

}

#endif
