#include "camera_perspective.hpp"
#include "base/json/json.hpp"
#include "base/math/math.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/plane.inl"
#include "base/math/sampling.inl"
#include "base/math/vector4.inl"
#include "rendering/sensor/sensor.hpp"
#include "sampler/camera_sample.hpp"
#include "sampler/sampler.hpp"
#include "scene/entity/composed_transformation.inl"
#include "scene/prop/prop.hpp"
#include "scene/prop/prop_intersection.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/scene_worker.hpp"

namespace scene::camera {

Perspective::Perspective(int2 resolution) noexcept
    : Camera(resolution), fov_(degrees_to_radians(60.f)) {}

uint32_t Perspective::num_views() const noexcept {
    return 1;
}

int2 Perspective::sensor_dimensions() const noexcept {
    return resolution_;
}

int4 Perspective::view_bounds(uint32_t /*view*/) const noexcept {
    return int4(int2(0, 0), resolution_ - int2(1));
}

float Perspective::pixel_solid_angle() const noexcept {
    return fov_ / static_cast<float>(resolution_[0]);
}

bool Perspective::generate_ray(Prop const* self, Camera_sample const& sample, uint32_t frame,
                               uint32_t /*view*/, Scene const& scene, Ray& ray) const noexcept {
    float2 const coordinates = float2(sample.pixel) + sample.pixel_uv;

    float3 direction = left_top_ + coordinates[0] * d_x_ + coordinates[1] * d_y_;

    float3 origin;

    if (lens_radius_ > 0.f) {
        float2 const lens = sample_disk_concentric(sample.lens_uv);

        origin = float3(lens_radius_ * lens, 0.f);

        float const t = focus_distance_ / direction[2];

        float3 const focus = t * direction;

        direction = focus - origin;
    } else {
        origin = float3(0.f);
    }

    uint64_t const time = absolute_time(frame, sample.time);

    Transformation temp;
    auto const&    transformation = self->transformation_at(entity_, time, temp, scene);

    float3 const origin_w = transformation.object_to_world_point(origin);

    direction                = normalize(direction);
    float3 const direction_w = transformation.object_to_world_vector(direction);

    ray = create_ray(origin_w, direction_w, time);

    return true;
}

bool Perspective::sample(Prop const* self, int4 const& bounds, uint64_t time, float3 const& p,
                         Sampler& sampler, uint32_t sampler_dimension, Scene const& scene,
                         Camera_sample_to& sample) const noexcept {
    Transformation temp;
    auto const&    transformation = self->transformation_at(entity_, time, temp, scene);

    float3 const po = transformation.world_to_object_point(p);

    float t;

    float3 dir;

    float3 out_dir;

    if (lens_radius_ > 0.f) {
        float2 const uv = sampler.generate_sample_2D(sampler_dimension);

        float2 const lens = sample_disk_concentric(uv);

        float3 const origin = float3(lens_radius_ * lens, 0.f);

        float3 const axis = po - origin;

        float const d = focus_distance_ / axis[2];

        dir     = origin + d * axis;
        t       = length(axis);
        out_dir = axis / t;
    } else {
        t       = length(po);
        dir     = po / t;
        out_dir = dir;
    }

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

    int2 const pixel(static_cast<int32_t>(fx), static_cast<int32_t>(fy));

    if (static_cast<uint32_t>(pixel[0] - bounds[0]) > static_cast<uint32_t>(bounds[2]) ||
        static_cast<uint32_t>(pixel[1] - bounds[1]) > static_cast<uint32_t>(bounds[3])) {
        return false;
    }

    float const cos_theta_2 = cos_theta * cos_theta;

    float const wa = 1.f / ((t * t) / cos_theta);
    float const wb = 1.f / (a_ * (cos_theta_2 * cos_theta_2));

    sample.pixel    = pixel;
    sample.pixel_uv = float2(x - fx, y - fy);
    sample.dir      = transformation.object_to_world_vector(out_dir);
    sample.t        = t;
    sample.pdf      = (wa * wb);

    return true;
}

void Perspective::set_fov(float fov) noexcept {
    fov_ = fov;
}

void Perspective::set_lens(Lens const& lens) noexcept {
    float const a = degrees_to_radians(lens.angle);
    float const c = std::cos(a);
    float const s = std::sin(a);

    float3 const axis(c, s, 0.f);
    float const  tilt = degrees_to_radians(lens.tilt);
    set_rotation(lens_tilt_, axis, tilt);

    float const shift = 2.f * lens.shift;

    lens_shift_  = float2(-s * shift, c * shift);
    lens_radius_ = lens.radius;
}

void Perspective::set_focus(Focus const& focus) noexcept {
    focus_ = focus;

    focus_.point[0] *= static_cast<float>(resolution_[0]);
    focus_.point[1] *= static_cast<float>(resolution_[1]);

    focus_distance_ = focus_.distance;
}

void Perspective::on_update(Prop const* self, uint64_t time, Worker& worker) noexcept {
    float2 const fr(resolution_);
    float const  ratio = fr[1] / fr[0];

    float const z = 1.f / std::tan(0.5f * fov_);

    //	float3 left_top   (-1.f,  ratio, z);
    //	float3 right_top  ( 1.f,  ratio, z);
    //	float3 left_bottom(-1.f, -ratio, z);

    float3 left_top    = transform_vector(lens_tilt_, float3(-1.f, ratio, 0.f));
    float3 right_top   = transform_vector(lens_tilt_, float3(1.f, ratio, 0.f));
    float3 left_bottom = transform_vector(lens_tilt_, float3(-1.f, -ratio, 0.f));

    left_top[2] += z;
    right_top[2] += z;
    left_bottom[2] += z;

    left_top_ = left_top + float3(lens_shift_, 0.f);
    d_x_      = (right_top - left_top) / fr[0];
    d_y_      = (left_bottom - left_top) / fr[1];

    update_focus(self, time, worker);

    float3 const nlb = left_bottom / z;
    float3 const nrt = right_top / z;

    a_ = std::abs((nrt[0] - nlb[0]) * (nrt[1] - nlb[1]));
}

void Perspective::update_focus(Prop const* self, uint64_t time, Worker& worker) noexcept {
    if (focus_.use_point && lens_radius_ > 0.f) {
        float3 const direction = normalize(left_top_ + focus_.point[0] * d_x_ +
                                           focus_.point[1] * d_y_);

        Transformation temp;
        auto const& transformation = self->transformation_at(entity_, time, temp, worker.scene());

        Ray ray(transformation.position, transformation.object_to_world_vector(direction), 0.f,
                Ray_max_t, 0, time, 0.f);

        prop::Intersection intersection;
        if (worker.intersect(ray, intersection)) {
            focus_distance_ = ray.max_t + focus_.point[2];
        } else {
            focus_distance_ = focus_.distance;
        }
    }
}

void Perspective::set_parameter(std::string_view name, json::Value const& value) noexcept {
    if ("fov" == name) {
        set_fov(degrees_to_radians(json::read_float(value)));
    } else if ("lens" == name) {
        Lens lens;
        load_lens(value, lens);
        set_lens(lens);
    } else if ("focus" == name) {
        Focus focus;
        load_focus(value, focus);
        set_focus(focus);
    }
}

void Perspective::load_lens(json::Value const& lens_value, Lens& lens) noexcept {
    for (auto const& n : lens_value.GetObject()) {
        if ("angle" == n.name) {
            lens.angle = json::read_float(n.value);
        } else if ("shift" == n.name) {
            lens.shift = json::read_float(n.value);
        } else if ("tilt" == n.name) {
            lens.tilt = json::read_float(n.value);
        } else if ("radius" == n.name) {
            lens.radius = json::read_float(n.value);
        }
    }
}

void Perspective::load_focus(json::Value const& focus_value, Focus& focus) noexcept {
    focus.use_point = false;

    for (auto const& n : focus_value.GetObject()) {
        if ("point" == n.name) {
            focus.point     = json::read_float3(n.value);
            focus.use_point = true;
        } else if ("distance" == n.name) {
            focus.distance = json::read_float(n.value);
        }
    }
}

}  // namespace scene::camera
