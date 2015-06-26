#pragma once

#include "scene/entity/entity.hpp"
#include "base/math/ray.hpp"

namespace sampler {

struct Camera_sample;

}

namespace rendering {

namespace film {

class Film;

}}

namespace scene { namespace camera {

class Camera : public entity::Entity {
public:

	Camera(const math::float2& dimensions, rendering::film::Film* film, float shutter_speed);
	virtual ~Camera();

	rendering::film::Film& film() const;

	virtual void update_view() = 0;

	virtual void generate_ray(const sampler::Camera_sample& sample, float tick_length, math::Oray& ray) const = 0;

protected:

	virtual void on_set_transformation() final override;

	static math::float2 calculate_dimensions(const math::float2& dimensions, rendering::film::Film* film);

	math::float2 dimensions_;
	rendering::film::Film* film_;
	float shutter_speed_;
};

}}
