#ifndef SU_CORE_RENDERING_SENSOR_SENSOR_HPP
#define SU_CORE_RENDERING_SENSOR_SENSOR_HPP

#include <cstddef>
#include "base/math/vector2.hpp"
#include "image/typed_image_fwd.hpp"

namespace thread {
class Pool;
}

namespace sampler {
struct Camera_sample;
struct Camera_sample_to;
}  // namespace sampler

namespace rendering::sensor {

class Sensor {
  public:
    using Camera_sample    = sampler::Camera_sample;
    using Camera_sample_to = sampler::Camera_sample_to;

    Sensor(float exposure);

    virtual ~Sensor();

    int2 dimensions() const;

    void resolve(thread::Pool& threads, image::Float4& target) const;

    void resolve_accumulate(thread::Pool& threads, image::Float4& target) const;

    void resize(int2 dimensions, int32_t num_layers);

    virtual int32_t filter_radius_int() const = 0;

    virtual int4 isolated_tile(int4 const& tile) const = 0;

    virtual void set_layer(int32_t layer) = 0;

    virtual void clear(float weight) = 0;

    virtual void set_weights(float weight) = 0;

    virtual void add_sample(Camera_sample const& sample, float4 const& color, int4 const& isolated,
                            int4 const& bounds) = 0;

    virtual void splat_sample(Camera_sample_to const& sample, float4 const& color,
                              int4 const& bounds) = 0;

    virtual bool has_alpha_transparency() const = 0;

  protected:
    virtual void add_pixel(int2 pixel, float4 const& color, float weight) = 0;

    virtual void add_pixel_atomic(int2 pixel, float4 const& color, float weight) = 0;

    virtual void splat_pixel_atomic(int2 pixel, float4 const& color, float weight) = 0;

    virtual void resolve(int32_t begin, int32_t end, image::Float4& target) const = 0;

    virtual void resolve_accumulate(int32_t begin, int32_t end, image::Float4& target) const = 0;

    virtual void on_resize(int2 dimensions, int32_t num_layers) = 0;

    int2 dimensions_;

    int32_t num_layers_;

    float exposure_factor_;
};

}  // namespace rendering::sensor

#endif
