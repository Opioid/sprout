#include "perspective_camera.hpp"

namespace camera {

Perspective::Perspective(film::Film* film) : Camera(film) {}

void Perspective::generate_ray(const sampler::Camera_sample& sample) const {

}

}
