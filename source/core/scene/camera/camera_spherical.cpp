#include "camera_spherical.hpp"
#include "base/math/math.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/sampling.inl"
#include "base/math/vector4.inl"
#include "rendering/sensor/sensor.hpp"
#include "sampler/camera_sample.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "scene/prop/prop.hpp"
#include "scene/scene.inl"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"

namespace scene::camera {

uint32_t Spherical::num_views() const {
    return 1;
}

int2 Spherical::sensor_dimensions() const {
    return resolution_;
}

int4 Spherical::view_bounds(uint32_t /*view*/) const {
    return int4(int2(0, 0), resolution_ - int2(1, 1));
}

float Spherical::pixel_solid_angle() const {
    return 1.f;
}

bool Spherical::generate_ray(Sample const& sample, uint32_t frame, uint32_t /*view*/,
                             Scene const& scene, Ray& ray) const {
    float2 coordinates = float2(sample.pixel) + sample.pixel_uv;

    float x = d_x_ * coordinates[0];
    float y = d_y_ * coordinates[1];

    float phi   = (x - 0.5f) * (2.f * Pi);
    float theta = y * Pi;

    float sin_phi   = std::sin(phi);
    float cos_phi   = std::cos(phi);
    float sin_theta = std::sin(theta);
    float cos_theta = std::cos(theta);

    float3 dir(sin_phi * sin_theta, cos_theta, cos_phi * sin_theta);

    uint64_t const time = absolute_time(frame, sample.time);

    Transformation temp;
    auto&          transformation = scene.prop_transformation_at(entity_, time, temp);

    ray = create_ray(transformation.position, transform_vector(transformation.rotation, dir), time);

    return true;
}

bool Spherical::sample(int4 const& /*bounds*/, uint64_t /*time*/, float3 const& /*p*/,
                       Sampler& /*sampler*/, uint32_t /*sampler_dimension*/, Scene const& /*scene*/,
                       Sample_to& /*sample*/) const {
    return false;
}

void Spherical::on_update(uint64_t /*time*/, Worker& /*worker*/) {
    float2 const fr(resolution_);
    d_x_ = 1.f / fr[0];
    d_y_ = 1.f / fr[1];
}

void Spherical::set_parameter(std::string_view /*name*/, json::Value const& /*value*/) {}

}  // namespace scene::camera
