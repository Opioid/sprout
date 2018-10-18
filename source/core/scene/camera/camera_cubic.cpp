#include "camera_cubic.hpp"
#include "base/math/math.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/sampling.inl"
#include "base/math/vector4.inl"
#include "rendering/sensor/sensor.hpp"
#include "sampler/camera_sample.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"

namespace scene::camera {

Cubic::Cubic(Layout layout, int2 resolution) noexcept : Camera(int2(resolution[0], resolution[1])) {
    float f = static_cast<float>(resolution[0]);

    left_top_ = float3(-1.f, 1.f, 1.f);

    float3 right_top(1.f, 1.f, 1.f);
    float3 left_bottom(-1.f, -1.f, 1.f);

    d_x_ = (right_top - left_top_) / f;
    d_y_ = (left_bottom - left_top_) / f;

    if (Layout::xmxymyzmz == layout) {
        for (int32_t i = 0; i < 6; ++i) {
            int2 offset = int2(resolution[0] * i, 0);

            view_bounds_[i] = int4{offset, offset + resolution_};
        }

        sensor_dimensions_ = int2(resolution_[0] * 6, resolution_[0]);
    } else if (Layout::xmxy_myzmz == layout) {
        int2 offset     = int2(resolution[0] * 0, 0);
        view_bounds_[0] = int4{offset, offset + resolution_};

        offset          = int2(resolution[0] * 1, 0);
        view_bounds_[1] = int4{offset, offset + resolution_};

        offset          = int2(resolution[0] * 2, 0);
        view_bounds_[2] = int4{offset, offset + resolution_};

        offset          = int2(resolution[0] * 0, resolution[0]);
        view_bounds_[3] = int4{offset, offset + resolution_};

        offset          = int2(resolution[0] * 1, resolution[0]);
        view_bounds_[4] = int4{offset, offset + resolution_};

        offset          = int2(resolution[0] * 2, resolution[0]);
        view_bounds_[5] = int4{offset, offset + resolution_};

        sensor_dimensions_ = int2(resolution_[0] * 3, resolution_[0] * 2);
    }

    math::set_rotation_y(view_rotations_[0], math::degrees_to_radians(-90.f));
    math::set_rotation_y(view_rotations_[1], math::degrees_to_radians(90.f));
    math::set_rotation_x(view_rotations_[2], math::degrees_to_radians(90.f));
    math::set_rotation_x(view_rotations_[3], math::degrees_to_radians(-90.f));
    view_rotations_[4] = float3x3::identity();
    math::set_rotation_y(view_rotations_[5], math::degrees_to_radians(180.f));
}

uint32_t Cubic::num_views() const noexcept {
    return 6;
}

int2 Cubic::sensor_dimensions() const noexcept {
    return sensor_dimensions_;
}

int4 Cubic::view_bounds(uint32_t view) const noexcept {
    return view_bounds_[view];
}

float Cubic::pixel_solid_angle() const noexcept {
    return 1.f;
}

bool Cubic::generate_ray(Camera_sample const& sample, uint32_t frame, uint32_t view, Ray& ray) const
    noexcept {
    float2 coordinates = float2(sample.pixel) + sample.pixel_uv;

    float3 direction = left_top_ + coordinates[0] * d_x_ + coordinates[1] * d_y_;

    direction = math::normalize(math::transform_vector(view_rotations_[view], direction));

    uint64_t const time = absolute_time(frame, sample.time);

    Transformation temp;
    auto const&    transformation = transformation_at(time, temp);

    ray = create_ray(math::transform_point(transformation.object_to_world, float3(0.f)),
                     math::transform_vector(transformation.object_to_world, direction), time);

    return true;
}

void Cubic::on_update(uint64_t /*time*/, Worker& /*worker*/) noexcept {}

void Cubic::set_parameter(std::string_view /*name*/, json::Value const& /*value*/) noexcept {}

}  // namespace scene::camera
