#include "camera_cubic_stereoscopic.hpp"
#include "base/json/json.hpp"
#include "base/math/math.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/sampling.inl"
#include "base/math/vector4.inl"
#include "rendering/sensor/sensor.hpp"
#include "sampler/camera_sample.hpp"
#include "scene/composed_transformation.inl"
#include "scene/prop/prop.hpp"
#include "scene/scene.inl"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"

namespace scene::camera {

Cubic_stereoscopic::Cubic_stereoscopic(Layout /*layout*/) {
    /*    float const f = float(resolution[0]);

        left_top_ = float3(-1.f, 1.f, 1.f);

        float3 const right_top(1.f, 1.f, 1.f);
        float3 const left_bottom(-1.f, -1.f, 1.f);

        d_x_ = (right_top - left_top_) / f;
        d_y_ = (left_bottom - left_top_) / f;

        if (Layout::lxlmxlylmylzlmzrxrmxryrmyrzrmz == layout) {
            for (int32_t i = 0; i < 12; ++i) {
                int2 offset = int2(resolution[0] * i, 0);

                view_bounds_[i] = int4(offset, offset + resolution_);
            }

            sensor_dimensions_ = int2(resolution_[0] * 12, resolution_[0]);
        } else if (Layout::rxlmxryrmyrzrmzlxlmxlylmylzlmz == layout) {
            for (int32_t i = 0; i < 6; ++i) {
                int2 offset = int2(resolution[0] * (i + 6), 0);

                view_bounds_[i] = int4(offset, offset + resolution_ - int2(1));
            }

            for (int32_t i = 6; i < 12; ++i) {
                int2 offset = int2(resolution[0] * (i - 6), 0);

                view_bounds_[i] = int4(offset, offset + resolution_ - int2(1));
            }

            sensor_dimensions_ = int2(resolution_[0] * 12, resolution_[0]);
        }

        math::set_rotation_y(view_rotations_[0], math::degrees_to_radians(-90.f));
        math::set_rotation_y(view_rotations_[1], math::degrees_to_radians(90.f));
        math::set_rotation_x(view_rotations_[2], math::degrees_to_radians(90.f));
        math::set_rotation_x(view_rotations_[3], math::degrees_to_radians(-90.f));
        view_rotations_[4] = float3x3::identity();
        math::set_rotation_y(view_rotations_[5], math::degrees_to_radians(180.f));

        set_interpupillary_distance_falloff(0.36f);
    */
}

uint32_t Cubic_stereoscopic::num_views() const {
    return 12;
}

int2 Cubic_stereoscopic::sensor_dimensions() const {
    return sensor_dimensions_;
}

int2 Cubic_stereoscopic::view_offset(uint32_t view) const {
    return view_offsets_[view];
}

float Cubic_stereoscopic::pixel_solid_angle() const {
    return 1.f;
}

bool Cubic_stereoscopic::generate_ray(Sample const& sample, uint32_t frame, uint32_t view,
                                      Scene const& scene, Ray& ray) const {
    float2 const coordinates = float2(sample.pixel) + sample.pixel_uv;

    float3 direction = left_top_ + coordinates[0] * d_x_ + coordinates[1] * d_y_;

    uint32_t const face = view % 6;

    direction = normalize(transform_vector(view_rotations_[face], direction));

    float const a = -std::atan2(direction[0], direction[2]);

    float3x3 rotation;
    math::set_rotation_y(rotation, a);

    float const ipd_scale = 1.f - std::pow(std::abs(direction[1]), 12.f - ipd_falloff_ * 10.f);

    uint32_t const eye = view < 6 ? 0 : 1;

    float3 const eye_offset = transform_vector(rotation, ipd_scale * eye_offsets_[eye]);

    uint64_t const time = absolute_time(frame, sample.time);

    Transformation temp;
    auto const&    trafo = scene.prop_transformation_at(entity_, time, temp);

    ray = create_ray(trafo.object_to_world_point(eye_offset),
                     trafo.object_to_world_vector(direction), time);

    return true;
}

bool Cubic_stereoscopic::sample(uint32_t /*view*/, int4_p /*bounds*/, uint64_t /*time*/,
                                float3_p /*p*/, Sampler& /*sampler*/, rnd::Generator& /*rng*/,
                                uint32_t /*sampler_d*/, Scene const& /*scene*/,
                                Sample_to& /*sample*/) const {
    return false;
}

void Cubic_stereoscopic::set_interpupillary_distance_falloff(float ipd_falloff) {
    ipd_falloff_ = std::sqrt(ipd_falloff);
}

void Cubic_stereoscopic::on_update(uint64_t /*time*/, Worker& /*worker*/) {}

void Cubic_stereoscopic::set_parameter(std::string_view name, json::Value const& value) {
    if ("stereo" == name) {
        for (auto n = value.MemberBegin(); n != value.MemberEnd(); ++n) {
            std::string const  node_name  = n->name.GetString();
            json::Value const& node_value = n->value;

            if ("ipd" == node_name) {
                set_interpupillary_distance(json::read_float(node_value));
            } else if ("ipd_falloff" == node_name) {
                set_interpupillary_distance_falloff(json::read_float(node_value));
            }
        }
    }
}

}  // namespace scene::camera
