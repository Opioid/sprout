#ifndef SU_CORE_SCENE_CAMERA_SPHERCICAL_STEREOSCOPIC_HPP
#define SU_CORE_SCENE_CAMERA_SPHERCICAL_STEREOSCOPIC_HPP

#include "camera_stereoscopic.hpp"

namespace scene::camera {

class Spherical_stereoscopic : public Stereoscopic {

public:

	Spherical_stereoscopic(int2 resolution);

	virtual uint32_t num_views() const override final;

	virtual int2 sensor_dimensions() const override final;

	virtual int4 view_bounds(uint32_t view) const override final;

	virtual float pixel_solid_angle() const override final;

	virtual bool generate_ray(sampler::Camera_sample const& sample, uint32_t view,
							  scene::Ray& ray) const override final;

private:

	virtual void on_update(Worker& worker) override final;

	virtual void set_parameter(std::string const& name, const json::Value& value) override final;

	float d_x_;
	float d_y_;

	int4 view_bounds_[2];
};

}

#endif
