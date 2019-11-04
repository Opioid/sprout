#include "camera_baking.hpp"
#include "base/math/math.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/sampling.inl"
#include "base/math/vector4.inl"
#include "rendering/sensor/sensor.hpp"
#include "sampler/camera_sample.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "scene/prop/prop.hpp"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"

namespace scene::camera {

Baking::Baking() noexcept : Camera(int2(0)) {}

uint32_t Baking::num_views() const noexcept {
    return 1;
}

int2 Baking::sensor_dimensions() const noexcept {
    return resolution_;
}

int4 Baking::view_bounds(uint32_t /*view*/) const noexcept {
    return int4(int2(0, 0), resolution_ - int2(1, 1));
}

float Baking::pixel_solid_angle() const noexcept {
    return 1.f;
}

bool Baking::generate_ray(Camera_sample const& /*sample*/, uint32_t /*frame*/, uint32_t /*view*/,
                          Scene const& /*scene*/, Ray& /*ray*/) const noexcept {
    return false;
}

bool Baking::sample(int4 const& /*bounds*/, uint64_t /*time*/, float3 const& /*p*/,
                    Sampler& /*sampler*/, uint32_t /*sampler_dimension*/, Scene const& /*scene*/,
                    Camera_sample_to& /*sample*/) const noexcept {
    return false;
}

void Baking::on_update(uint64_t /*time*/, Worker& /*worker*/) noexcept {}

void Baking::set_parameter(std::string_view /*name*/, json::Value const& /*value*/) noexcept {}

}  // namespace scene::camera
