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

    Perspective();

    uint32_t num_views() const final;

    int2 sensor_dimensions() const final;

    int2 view_offset(uint32_t view) const final;

    float pixel_solid_angle() const final;

    bool generate_ray(Sample const& sample, uint32_t frame, uint32_t view, Scene const& scene,
                      Ray& ray) const final;

    bool sample(uint32_t view, int4 const& bounds, uint64_t time, float3 const& p, Sampler& sampler,
                rnd::Generator& rng, uint32_t sampler_dimension, Scene const& scene,
                Sample_to& sample) const final;

    Ray_differential calculate_ray_differential(float3 const& p, uint64_t time,
                                                Scene const& scene) const final;

    Frustum frustum() const final;

    void set_fov(float fov);

    void set_lens(Lens const& lens);

    void set_focus(Focus const& focus);

  private:
    void on_update(uint64_t time, Worker& worker) final;

    void update_focus(uint64_t time, Worker& worker);

    void set_parameter(std::string_view name, json::Value const& value) final;

    void load_lens(json::Value const& lens_value, Lens& lens);

    void load_focus(json::Value const& focus_value, Focus& focus);

    float3 left_top_;
    float3 d_x_;
    float3 d_y_;

    float3x3 lens_tilt_;
    float2   lens_shift_;
    float    lens_radius_;

    float fov_;

    Focus focus_;
    float focus_distance_;

    float a_;

    Frustum frustum_;
};

}  // namespace scene::camera

#endif
