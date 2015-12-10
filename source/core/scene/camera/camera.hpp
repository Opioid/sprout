#pragma once

#include "scene/entity/entity.hpp"
#include "base/math/ray.hpp"

namespace sampler { struct Camera_sample; }

namespace rendering {

class Worker;

namespace sensor { class Sensor; }

}

namespace scene { namespace camera {

class Camera : public entity::Entity {
public:

	Camera(math::uint2 resolution, float ray_max_t, float frame_duration, bool motion_blur);

	virtual ~Camera();

	virtual uint32_t num_views() const = 0;

	virtual math::uint2 sensor_dimensions() const = 0;

	virtual math::uint2 sensor_pixel(math::uint2 pixel, uint32_t view) const = 0;

	virtual void update_focus(rendering::Worker& worker) = 0;

	virtual void generate_ray(const sampler::Camera_sample& sample, uint32_t view, math::Oray& ray) const = 0;

	math::uint2 resolution() const;

	rendering::sensor::Sensor& sensor() const;
	void set_sensor(rendering::sensor::Sensor* sensor);

	math::uint2 seed(uint32_t x, uint32_t y) const;
	void set_seed(uint32_t x, uint32_t y, math::uint2 seed);

	float frame_duration() const;

	bool motion_blur() const;

protected:

	virtual void on_set_transformation() final override;

	math::uint2 resolution_;
	rendering::sensor::Sensor* sensor_;

	math::uint2* seeds_;

	float ray_max_t_;
	float frame_duration_;
	bool motion_blur_;
};

}}
