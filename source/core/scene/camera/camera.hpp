#pragma once

#include "scene/entity/entity.hpp"

namespace film {

class Film;

}

namespace sampler {

struct Camera_sample;

}

namespace camera {

class Camera : public scene::Entity {
public:

	Camera(film::Film* film);
	virtual ~Camera();

	film::Film& film() const;

	virtual void generate_ray(const sampler::Camera_sample& sample) const = 0;

protected:

	film::Film* film_;
};

}
