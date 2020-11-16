#include "camera_perspective_stereoscopic.hpp"
#include "base/json/json.hpp"
#include "base/math/math.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/sampling.inl"
#include "base/math/vector4.inl"
#include "rendering/sensor/sensor.hpp"
#include "sampler/camera_sample.hpp"
#include "scene/entity/composed_transformation.inl"
#include "scene/prop/prop.hpp"
#include "scene/scene.inl"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"

namespace scene::camera {

Perspective_stereoscopic::Perspective_stereoscopic() : fov_(degrees_to_radians(90.f)) {}

uint32_t Perspective_stereoscopic::num_views() const {
    return 2;
}

int2 Perspective_stereoscopic::sensor_dimensions() const {
    return int2(resolution_[0] * 2, resolution_[1]);
}

int2 Perspective_stereoscopic::view_offset(uint32_t view) const {
    return view_offsets_[view];
}

float Perspective_stereoscopic::pixel_solid_angle() const {
    // Assume square pixels...
    float const x = fov_ / float(resolution_[0]);

    return x * x;
}

bool Perspective_stereoscopic::generate_ray(Sample const& sample, uint32_t frame, uint32_t view,
                                            Scene const& scene, scene::Ray& ray) const {
    float2 const coordinates = float2(sample.pixel) + sample.pixel_uv;

    float3 direction = left_top_ + coordinates[0] * d_x_ + coordinates[1] * d_y_;
    direction        = normalize(direction);

    uint64_t const time = absolute_time(frame, sample.time);

    Transformation temp;
    auto&          trafo = scene.prop_transformation_at(entity_, time, temp);

    ray = create_ray(trafo.object_to_world_point(eye_offsets_[view]),
                     trafo.object_to_world_vector(direction), time);

    return true;
}

bool Perspective_stereoscopic::sample(uint32_t view, int4_p bounds, uint64_t time, float3_p p,
                                      Sampler& /*sampler*/, rnd::Generator& /*rng*/,
                                      uint32_t /*sampler_d*/, Scene const& scene,
                                      Sample_to& sample) const {
    Transformation temp;
    auto const&    trafo = scene.prop_transformation_at(entity_, time, temp);

    float3 const po = trafo.world_to_object_point(p) - eye_offsets_[view];

    float t;

    float3 dir;

    float3 out_dir;

    t       = length(po);
    dir     = po / t;
    out_dir = dir;

    float const cos_theta = out_dir[2];

    if (cos_theta < 0.f) {
        return false;
    }

    float3 const pd = left_top_[2] * (dir / dir[2]);

    float3 const offset = pd - left_top_;

    float const x = offset[0] / d_x_[0];
    float const y = offset[1] / d_y_[1];

    float const fx = std::floor(x);
    float const fy = std::floor(y);

    int2 const pixel(fx, fy);

    if (uint32_t(pixel[0] - bounds[0]) > uint32_t(bounds[2]) ||
        uint32_t(pixel[1] - bounds[1]) > uint32_t(bounds[3])) {
        return false;
    }

    float const cos_theta_2 = cos_theta * cos_theta;

    float const wa = 1.f / ((t * t) / cos_theta);
    float const wb = 1.f / (a_ * (cos_theta_2 * cos_theta_2));

    sample.pixel    = pixel;
    sample.pixel_uv = float2(x - fx, y - fy);
    sample.dir      = trafo.object_to_world_vector(out_dir);
    sample.t        = t;
    sample.pdf      = wa * wb;

    return true;
}

void Perspective_stereoscopic::set_fov(float fov) {
    fov_ = fov;
}

void Perspective_stereoscopic::on_update(uint64_t /*time*/, Worker& /*worker*/) {
    view_offsets_[0] = int2(0);
    view_offsets_[1] = int2(resolution_[0], 0);

    float2 const fr(resolution_);
    float const  ratio = fr[0] / fr[1];

    float const z = 1.f / std::tan(0.5f * fov_);

    left_top_ = float3(-ratio, 1.f, z);
    float3 const right_top(ratio, 1.f, z);
    float3 const left_bottom(-ratio, -1.f, z);

    d_x_ = (right_top - left_top_) / fr[0];
    d_y_ = (left_bottom - left_top_) / fr[1];

    float3 const nlb = left_bottom / z;
    float3 const nrt = right_top / z;

    a_ = std::abs((nrt[0] - nlb[0]) * (nrt[1] - nlb[1]));
}

void Perspective_stereoscopic::set_parameter(std::string_view name, json::Value const& value) {
    if ("fov" == name) {
        set_fov(degrees_to_radians(json::read_float(value)));
    } else if ("stereo" == name) {
        set_interpupillary_distance(json::read_float(value, "ipd", 0.062f));
    }
}

}  // namespace scene::camera
