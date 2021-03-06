#include "camera_spherical_stereoscopic.hpp"
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

// the layout should match
// https://developers.google.com/vr/jump/rendering-ods-content.pdf

namespace scene::camera {

uint32_t Spherical_stereoscopic::num_views() const {
    return 2;
}

int2 Spherical_stereoscopic::sensor_dimensions() const {
    return int2(resolution_[0], resolution_[1] * 2);
}

int2 Spherical_stereoscopic::view_offset(uint32_t view) const {
    return view_offsets_[view];
}

float Spherical_stereoscopic::pixel_solid_angle() const {
    return 1.f;
}

bool Spherical_stereoscopic::generate_ray(Sample const& sample, uint32_t frame, uint32_t view,
                                          Scene const& scene, Ray& ray) const {
    float2 const coordinates = float2(sample.pixel) + sample.pixel_uv;

    float const x = d_x_ * coordinates[0];
    float const y = d_y_ * coordinates[1];

    float const phi   = (x - 0.5f) * (2.f * Pi);
    float const theta = y * Pi;

    float const sin_phi   = std::sin(phi);
    float const cos_phi   = std::cos(phi);
    float const sin_theta = std::sin(theta);
    float const cos_theta = std::cos(theta);

    float3 const dir(sin_phi * sin_theta, cos_theta, cos_phi * sin_theta);

    float3x3 rotation;
    math::set_rotation_y(rotation, phi);
    float3 const eye_pos = transform_vector(rotation, eye_offsets_[view]);

    uint64_t const time = absolute_time(frame, sample.time);

    Transformation temp;
    auto const&    trafo = scene.prop_transformation_at(entity_, time, temp);

    ray = create_ray(trafo.object_to_world_point(eye_pos), transform_vector(trafo.rotation, dir),
                     time);

    return true;
}

bool Spherical_stereoscopic::sample(uint32_t /*view*/, int4_p /*bounds*/, uint64_t /*time*/,
                                    float3_p /*p*/, Sampler& /*sampler*/, rnd::Generator& /*rng*/,
                                    uint32_t /*sampler_d*/, Scene const& /*scene*/,
                                    Sample_to& /*sample*/) const {
    return false;
}

void Spherical_stereoscopic::on_update(uint64_t /*time*/, Worker& /*worker*/) {
    float2 const fr(resolution_);
    d_x_ = 1.f / fr[0];
    d_y_ = 1.f / fr[1];

    view_offsets_[0] = int2(0);
    view_offsets_[1] = int2(0, resolution_[1]);
}

void Spherical_stereoscopic::set_parameter(std::string_view name, json::Value const& value) {
    if ("stereo" == name) {
        for (auto n = value.MemberBegin(); n != value.MemberEnd(); ++n) {
            std::string const  node_name  = n->name.GetString();
            json::Value const& node_value = n->value;

            if ("ipd" == node_name) {
                set_interpupillary_distance(json::read_float(node_value));
            }
        }
    }
}

}  // namespace scene::camera
