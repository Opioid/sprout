#ifndef SU_CORE_RENDERING_SENSOR_SENSOR_HPP
#define SU_CORE_RENDERING_SENSOR_SENSOR_HPP

#include "base/math/vector2.hpp"
#include "image/typed_image_fwd.hpp"
#include "aov/buffer.hpp"

namespace thread {
class Pool;
}

using Threads = thread::Pool;

namespace sampler {
struct Camera_sample;
struct Camera_sample_to;
}  // namespace sampler

namespace rendering::sensor {

namespace aov {
class Value;
}

class Sensor {
  public:
    using Camera_sample    = sampler::Camera_sample;
    using Camera_sample_to = sampler::Camera_sample_to;

    Sensor(int32_t filter_radius);

    virtual ~Sensor();

    int2 dimensions() const;

    void resolve(Threads& threads, image::Float4& target) const;

    void resolve_accumulate(Threads& threads, image::Float4& target) const;

    void resolve(aov::Property aov, Threads& threads, image::Float4& target) const;

    void resize(int2 dimensions, int32_t num_layers);

    int32_t filter_radius_int() const;

    int4 isolated_tile(int4 const& tile) const;

    virtual void set_layer(int32_t layer) = 0;

    void clear(float weight);

    virtual void set_weights(float weight) = 0;

    virtual void fix_zero_weights() = 0;

    virtual void add_sample(Camera_sample const& sample, float4 const& color, aov::Value const& aov, int4 const& isolated,
                            int2 offset, int4 const& bounds) = 0;

    virtual void splat_sample(Camera_sample_to const& sample, float4 const& color, int2 offset,
                              int4 const& bounds) = 0;

    virtual bool has_alpha_transparency() const = 0;

  protected:
    virtual void add_pixel(int2 pixel, float4 const& color, float weight) = 0;

    void add_pixel(int2 pixel, float4 const& value, float weight, aov::Property aov);

    virtual void add_pixel_atomic(int2 pixel, float4 const& color, float weight) = 0;

    void add_pixel_atomic(int2 pixel, float4 const& value, float weight, aov::Property aov);

    virtual void splat_pixel_atomic(int2 pixel, float4 const& color, float weight) = 0;

    virtual void resolve(int32_t begin, int32_t end, image::Float4& target) const = 0;

    virtual void resolve_accumulate(int32_t begin, int32_t end, image::Float4& target) const = 0;

    void resolve(int32_t begin, int32_t end, aov::Property aov, image::Float4& target) const;

    virtual void on_resize(int2 dimensions, int32_t num_layers) = 0;

    virtual void on_clear(float weight) = 0;

    int2 dimensions_;

    int32_t const filter_radius_;

    int32_t num_layers_;

    aov::Buffer aov_;
};

}  // namespace rendering::sensor

#endif
