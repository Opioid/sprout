#ifndef SU_CORE_SCENE_CAMERA_PERSPECTIVE_STEREOSCOPIC_HPP
#define SU_CORE_SCENE_CAMERA_PERSPECTIVE_STEREOSCOPIC_HPP

#include "base/math/vector4.hpp"
#include "camera_stereoscopic.hpp"

namespace scene::camera {

class Perspective_stereoscopic final : public Stereoscopic {
  public:
    Perspective_stereoscopic();

    uint32_t num_views() const final;

    int2 sensor_dimensions() const final;

    int2 view_offset(uint32_t view) const final;

    float pixel_solid_angle() const final;

    bool generate_ray(Sample const& sample, uint32_t frame, uint32_t view, Scene const& scene,
                      Ray& ray) const final;

    bool sample(uint32_t view, int4_p bounds, uint64_t time, float3_p p, Sampler& sampler, RNG& rng,
                uint32_t sampler_d, Scene const& scene, Sample_to& sample) const final;

    void set_fov(float fov);

  private:
    void on_update(uint64_t time, Worker& worker) final;

    void set_parameter(std::string_view name, json::Value const& value) final;

    float3 left_top_;
    float3 d_x_;
    float3 d_y_;

    int2 view_offsets_[2];

    float fov_;

    float a_;
};

}  // namespace scene::camera

#endif
