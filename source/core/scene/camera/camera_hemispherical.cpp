#include "camera_hemispherical.hpp"
#include "base/math/mapping.inl"
#include "base/math/math.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/sampling/sampling.hpp"
#include "base/math/vector4.inl"
#include "rendering/sensor/sensor.hpp"
#include "sampler/camera_sample.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"

namespace scene::camera {

Hemispherical::Hemispherical(int2 resolution) noexcept : Camera(resolution) {
    float2 fr(resolution);
    d_x_ = 1.f / fr[0];
    d_y_ = 1.f / fr[1];
}

uint32_t Hemispherical::num_views() const noexcept {
    return 1;
}

int2 Hemispherical::sensor_dimensions() const noexcept {
    return resolution_;
}

int4 Hemispherical::view_bounds(uint32_t /*view*/) const noexcept {
    return int4(int2(0), resolution_ - int2(1));
}

float Hemispherical::pixel_solid_angle() const noexcept {
    return 1.f;
}

bool Hemispherical::generate_ray(Camera_sample const& sample, uint32_t /*view*/,
                                 Ray&                   ray) const noexcept {
    float2 coordinates = float2(sample.pixel) + sample.pixel_uv;

    float x = d_x_ * coordinates[0];
    float y = d_y_ * coordinates[1];

    x = 2.f * x - 1.f;
    y = 2.f * y - 1.f;

    float z = x * x + y * y;
    if (z > 1.f) {
        return false;
    }

    float3 dir = math::disk_to_hemisphere_equidistant(float2(x, y));

    Transformation temp;
    auto&                           transformation = transformation_at(sample.time, temp);

    ray.origin = transformation.position;
    ray.set_direction(math::transform_vector(transformation.rotation, dir));
    ray.min_t = 0.f;
    ray.max_t = Ray_max_t;
    ray.time  = sample.time;
    ray.depth = 0;

    return true;
}

void Hemispherical::on_update(Worker& /*worker*/) noexcept {}

void Hemispherical::set_parameter(std::string_view /*name*/, json::Value const& /*value*/) noexcept {}

}  // namespace scene::camera
