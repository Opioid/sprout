#include "camera_perspective_stereoscopic.hpp"
#include "base/json/json.hpp"
#include "base/math/math.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/sampling.inl"
#include "base/math/vector4.inl"
#include "rendering/sensor/sensor.hpp"
#include "sampler/camera_sample.hpp"
#include "scene/prop/prop.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"

namespace scene::camera {

Perspective_stereoscopic::Perspective_stereoscopic(int2 resolution) noexcept
    : Stereoscopic(resolution) {
    set_fov(90.f);

    view_bounds_[0] = int4(int2(0, 0), resolution - int2(1, 1));
    view_bounds_[1] = int4(int2(resolution[0], 0),
                           int2(resolution[0] * 2, resolution[1]) - int2(1));
}

uint32_t Perspective_stereoscopic::num_views() const noexcept {
    return 2;
}

int2 Perspective_stereoscopic::sensor_dimensions() const noexcept {
    return int2(resolution_[0] * 2, resolution_[1]);
}

int4 Perspective_stereoscopic::view_bounds(uint32_t view) const noexcept {
    return view_bounds_[view];
}

float Perspective_stereoscopic::pixel_solid_angle() const noexcept {
    return 1.f;
}

bool Perspective_stereoscopic::generate_ray(Prop const* self, sampler::Camera_sample const& sample,
                                            uint32_t frame, uint32_t view, Scene const& scene,
                                            scene::Ray& ray) const noexcept {
    float2 coordinates = float2(sample.pixel) + sample.pixel_uv;

    float3 direction = left_top_ + coordinates[0] * d_x_ + coordinates[1] * d_y_;
    direction        = normalize(direction);

    uint64_t const time = absolute_time(frame, sample.time);

    Transformation temp;
    auto&          transformation = self->transformation_at(entity_, time, temp, scene);

    ray = create_ray(transform_point(transformation.object_to_world, eye_offsets_[view]),
                     transform_vector(transformation.object_to_world, direction), time);

    return true;
}

bool Perspective_stereoscopic::sample(float3 const& /*p*/) const noexcept {
    return false;
}

void Perspective_stereoscopic::set_fov(float fov) noexcept {
    float2 fr(resolution_);
    float  ratio = fr[0] / fr[1];

    float z = ratio * Pi / fov * 0.5f;

    left_top_ = float3(-ratio, 1.f, z);
    float3 right_top(ratio, 1.f, z);
    float3 left_bottom(-ratio, -1.f, z);

    d_x_ = (right_top - left_top_) / fr[0];
    d_y_ = (left_bottom - left_top_) / fr[1];
}

void Perspective_stereoscopic::on_update(Prop const* /*self*/, uint64_t /*time*/,
                                         Worker& /*worker*/) noexcept {}

void Perspective_stereoscopic::set_parameter(std::string_view   name,
                                             json::Value const& value) noexcept {
    if ("fov" == name) {
        set_fov(math::degrees_to_radians(json::read_float(value)));
    } else if ("stereo" == name) {
        set_interpupillary_distance(json::read_float(value, "ipd", 0.062f));
    }
}

}  // namespace scene::camera
