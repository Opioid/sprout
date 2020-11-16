#ifndef SU_CORE_SCENE_CAMERA_CUBIC_STEREOSCOPIC_HPP
#define SU_CORE_SCENE_CAMERA_CUBIC_STEREOSCOPIC_HPP

#include "base/math/matrix3x3.hpp"
#include "camera_stereoscopic.hpp"

namespace scene::camera {

class Cubic_stereoscopic final : public Stereoscopic {
  public:
    enum class Layout {
        lxlmxlylmylzlmzrxrmxryrmyrzrmz,
        rxlmxryrmyrzrmzlxlmxlylmylzlmz,
    };

    Cubic_stereoscopic(Layout layout);

    uint32_t num_views() const final;

    int2 sensor_dimensions() const final;

    int2 view_offset(uint32_t view) const final;

    float pixel_solid_angle() const final;

    bool generate_ray(Sample const& sample, uint32_t frame, uint32_t view, Scene const& scene,
                      Ray& ray) const final;

    bool sample(uint32_t view, int4_p bounds, uint64_t time, float3_p p, Sampler& sampler, RNG& rng,
                uint32_t sampler_d, Scene const& scene, Sample_to& sample) const final;

    void set_interpupillary_distance_falloff(float ipd_falloff);

  private:
    void on_update(uint64_t time, Worker& worker) final;

    void set_parameter(std::string_view name, json::Value const& value) final;

    float3 left_top_;
    float3 d_x_;
    float3 d_y_;

    int2 sensor_dimensions_;

    float ipd_falloff_;

    int2 view_offsets_[12];

    float3x3 view_rotations_[6];
};

}  // namespace scene::camera

#endif
