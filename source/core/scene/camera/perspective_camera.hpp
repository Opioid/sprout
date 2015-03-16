#pragma once

#include "camera.hpp"

namespace camera {

class Perspective : public Camera {
public:

	Perspective(film::Film* film, float fov);

	virtual void generate_ray(const sampler::Camera_sample& sample, math::Oray& ray) const;

private:

	float fov_;
};

}
