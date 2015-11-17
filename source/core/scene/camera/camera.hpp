#pragma once

#include "scene/entity/entity.hpp"
#include "base/math/ray.hpp"

namespace sampler { struct Camera_sample; }

namespace rendering {

class Worker;

namespace film {

class Film;

}}

namespace scene { namespace camera {

class Camera : public entity::Entity {
public:

	Camera(const math::float2& dimensions, rendering::film::Film* film,
		   float frame_duration, bool motion_blur);

	virtual ~Camera();

	rendering::film::Film& film() const;

	float frame_duration() const;

	bool motion_blur() const;

	virtual void update_focus(rendering::Worker& worker) = 0;

	virtual void generate_ray(const sampler::Camera_sample& sample,
							  float normalized_tick_offset, float normalized_tick_slice,
							  math::Oray& ray) const = 0;

protected:

	virtual void on_set_transformation() final override;

	static math::float2 calculate_dimensions(const math::float2& dimensions, rendering::film::Film* film);

	math::float2 dimensions_;
	rendering::film::Film* film_;
	float frame_duration_;
	bool motion_blur_;
};

}}
