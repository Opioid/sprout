#ifndef SU_CORE_SCENE_CAMERA_PERSPECTIVE_HPP
#define SU_CORE_SCENE_CAMERA_PERSPECTIVE_HPP

#include "base/math/frustum.hpp"
#include "base/math/matrix3x3.hpp"
#include "camera.hpp"

namespace scene::camera {

class Perspective final : public Camera {
  public:
    struct Lens {
        float angle  = 0.f;
        float shift  = 0.f;
        float tilt   = 0.f;
        float radius = 0.f;
    };

    struct Focus {
        float3 point = float3(0.5f, 0.5f, 0.f);
        float  distance;

        bool use_point = false;
    };

    Perspective(int2 resolution) noexcept;

    uint32_t num_views() const noexcept override final;

    int2 sensor_dimensions() const noexcept override final;

    int4 view_bounds(uint32_t view) const noexcept override final;

    float pixel_solid_angle() const noexcept override final;

    bool generate_ray(Prop const* self, Camera_sample const& sample, uint32_t frame, uint32_t view,
                      Scene const& scene, Ray& ray) const noexcept override final;

    bool sample(Prop const* self, int4 const& bounds, uint64_t time, float3 const& p,
                Sampler& sampler, uint32_t sampler_dimension, Scene const& scene,
                Camera_sample_to& sample) const noexcept override final;

    Frustum frustum() const noexcept override final;

    void set_fov(float fov) noexcept;

    void set_lens(Lens const& lens) noexcept;

    void set_focus(Focus const& focus) noexcept;

  private:
    void on_update(Prop const* self, uint64_t time, Worker& worker) noexcept override final;

    void update_focus(Prop const* self, uint64_t time, Worker& worker) noexcept;

    void set_parameter(std::string_view name, json::Value const& value) noexcept override final;

    void load_lens(json::Value const& lens_value, Lens& lens) noexcept;

    void load_focus(json::Value const& focus_value, Focus& focus) noexcept;

    float3 left_top_;
    float3 d_x_;
    float3 d_y_;

    float3x3 lens_tilt_   = float3x3::identity();
    float2   lens_shift_  = float2(0.f, 0.f);
    float    lens_radius_ = 0.f;

    float fov_;

    Focus focus_;
    float focus_distance_;

    float a_;

    Frustum frustum_;
};

}  // namespace scene::camera

#endif
