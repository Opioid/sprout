#pragma once

namespace film {

class Film;

}

namespace sampler {

struct Camera_sample;

}

namespace camera {

class Camera {
public:

	Camera(film::Film* film);
	virtual ~Camera();

	film::Film& film() const;

	virtual void generate_ray(const sampler::Camera_sample& sample) const = 0;

protected:

	film::Film* film_;
};

}
