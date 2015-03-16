#pragma once

#include "scene/entity/entity.hpp"
#include "base/math/ray.hpp"

namespace film {

class Film;

}

namespace sampler {

struct Camera_sample;

}

namespace camera {

class Camera : public scene::Entity {
public:

	Camera(const math::float2& dimensions, film::Film* film);
	virtual ~Camera();

	film::Film& film() const;

	virtual void update_view() = 0;

	virtual void generate_ray(const sampler::Camera_sample& sample, math::Oray& ray) const = 0;

protected:

	static math::float2 calculate_dimensions(const math::float2& dimensions, film::Film* film);

	math::float2 dimensions_;
	film::Film* film_;
};

}
