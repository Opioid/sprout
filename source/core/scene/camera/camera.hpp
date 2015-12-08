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

	Camera(math::float2 dimensions, rendering::sensor::Sensor* sensor, float ray_max_t,
		   float frame_duration, bool motion_blur);

	virtual ~Camera();

	rendering::sensor::Sensor& sensor() const;

	float frame_duration() const;

	bool motion_blur() const;

	virtual void update_focus(rendering::Worker& worker) = 0;

	virtual void generate_ray(const sampler::Camera_sample& sample, math::Oray& ray) const = 0;

protected:

	virtual void on_set_transformation() final override;

	static math::float2 calculate_dimensions(math::float2 dimensions, rendering::sensor::Sensor* sensor);

	math::float2 dimensions_;
	rendering::sensor::Sensor* film_;
	float ray_max_t_;
	float frame_duration_;
	bool motion_blur_;
};

}}
