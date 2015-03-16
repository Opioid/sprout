#include "perspective_camera.hpp"

namespace camera {

Perspective::Perspective(film::Film* film, float fov) : Camera(film), fov_(fov) {}

void Perspective::generate_ray(const sampler::Camera_sample& sample, math::Oray& ray) const {

}

}
