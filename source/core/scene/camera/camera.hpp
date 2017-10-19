#pragma once

#include "scene/entity/entity.hpp"
#include "base/math/vector2.hpp"
#include "base/math/vector4.hpp"
#include <string>

namespace sampler { struct Camera_sample; }

namespace rendering {

class Worker;

namespace sensor { class Sensor; }

}

namespace scene {

struct Ray;

namespace camera {

class Camera : public entity::Entity {

public:

	Camera(int2 resolution);
	virtual ~Camera();

	virtual uint32_t num_views() const = 0;

	virtual int2 sensor_dimensions() const = 0;

	virtual int4 view_bounds(uint32_t view) const = 0;

	virtual float pixel_solid_angle() const = 0;

	void update(rendering::Worker& worker);

	virtual bool generate_ray(const sampler::Camera_sample& sample,
							  uint32_t view, scene::Ray& ray) const = 0;

	virtual void set_parameters(const json::Value& parameters) override final;

	int2 resolution() const;

	rendering::sensor::Sensor& sensor() const;
	void set_sensor(rendering::sensor::Sensor* sensor);

	float frame_duration() const;
	void set_frame_duration(float frame_duration);

	bool motion_blur() const;
	void set_motion_blur(bool motion_blur);

protected:

	virtual void on_update(rendering::Worker& worker) = 0;

	virtual void set_parameter(const std::string& name, const json::Value& value) = 0;

	virtual void on_set_transformation() override final;

	int2 resolution_;
	rendering::sensor::Sensor* sensor_ = nullptr;

	int32_t filter_radius_ = 0;

	float frame_duration_ = 0.f;
	bool motion_blur_ = true;
};

}}
