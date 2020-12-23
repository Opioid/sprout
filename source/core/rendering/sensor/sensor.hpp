#ifndef SU_CORE_RENDERING_SENSOR_SENSOR_HPP
#define SU_CORE_RENDERING_SENSOR_SENSOR_HPP

#include "aov/buffer.hpp"
#include "base/math/vector2.hpp"
#include "image/typed_image_fwd.hpp"

namespace thread {
class Pool;
}

using Threads = thread::Pool;

namespace sampler {
struct Camera_sample;
struct Camera_sample_to;
}  // namespace sampler

namespace rendering::sensor {

class Sensor {
  public:
    using Camera_sample    = sampler::Camera_sample;
    using Camera_sample_to = sampler::Camera_sample_to;
    using AOV              = rendering::sensor::aov::Property;

    Sensor(int32_t filter_radius, bool transparency, bool adaptive);

    virtual ~Sensor();

    int2 dimensions() const;

    int32_t filter_radius_int() const;

    bool alpha_transparency() const;

    bool adaptive() const;

    void resolve(Threads& threads, image::Float4& target) const;

    void resolve_accumulate(Threads& threads, image::Float4& target) const;

    void resolve(uint32_t slot, AOV property, Threads& threads, image::Float4& target) const;

    void resize(int2 dimensions, int32_t num_layers, aov::Value_pool const& aovs);

    static uint32_t num_samples_to_estimate(uint32_t max_samples);

    uint32_t num_samples_by_estimate(int2 pixel, uint32_t max_samples) const;

    void set_variance_estimate(int2 pixel, float variance);

    void normalize_variance_estimate(Threads& threads);

    int4 isolated_tile(int4_p tile) const;

    virtual void set_layer(int32_t layer) = 0;

    void clear(float weight, aov::Value_pool const& aovs);

    virtual void set_weights(float weight) = 0;

    virtual void fix_zero_weights() = 0;

    virtual float4 add_sample(Camera_sample const& sample, float4_p color, aov::Value const* aov,
                              int4_p isolated, int2 offset, int4_p bounds) = 0;

    virtual void splat_sample(Camera_sample_to const& sample, float4_p color, int2 offset,
                              int4_p bounds) = 0;

  protected:
    virtual void add_pixel(int2 pixel, float4_p color, float weight) = 0;

    virtual void add_pixel_atomic(int2 pixel, float4_p color, float weight) = 0;

    void add_AOV(int2 pixel, uint32_t slot, float4_p value, float weight);

    void add_AOV_atomic(int2 pixel, uint32_t slot, float4_p value, float weight);

    void overwrite_AOV(int2 pixel, uint32_t slot, float4_p value);

    void less_AOV(int2 pixel, uint32_t slot, float value);

    virtual void splat_pixel_atomic(int2 pixel, float4_p color, float weight) = 0;

    virtual void resolve(int32_t begin, int32_t end, image::Float4& target) const = 0;

    virtual void resolve_accumulate(int32_t begin, int32_t end, image::Float4& target) const = 0;

    virtual void resolve(int32_t begin, int32_t end, uint32_t slot, AOV property,
                         image::Float4& target) const = 0;

    virtual void on_resize(int2 dimensions, int32_t num_layers) = 0;

    virtual void on_clear(float weight) = 0;

    int2 dimensions_;

    int32_t const filter_radius_;

    bool const transparency_;
    bool const adaptive_;

    int16_t num_layers_;

    aov::Buffer aov_;

    int32_t variance_start_;
    float   max_variance_;

    float* variance_;
};

}  // namespace rendering::sensor

#endif
