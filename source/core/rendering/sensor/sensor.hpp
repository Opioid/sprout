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

    Sensor(int2 dimensions, float exposure) noexcept;

    virtual ~Sensor() noexcept;

    int2 dimensions() const noexcept;

    void resolve(thread::Pool& pool, image::Float4& target) const noexcept;

    void resolve_accumulate(thread::Pool& pool, image::Float4& target) const noexcept;

    virtual int32_t filter_radius_int() const noexcept = 0;

    virtual int4 isolated_tile(int4 const& tile) const noexcept = 0;

    virtual void clear(float weigth) = 0;

    virtual void add_sample(Camera_sample const& sample, float4 const& color, int4 const& isolated,
                            int4 const& bounds) noexcept = 0;

    virtual void add_sample(Camera_sample_to const& sample, float4 const& color,
                            int4 const& bounds) noexcept = 0;

    virtual bool has_alpha_transparency() const noexcept = 0;

    virtual size_t num_bytes() const noexcept = 0;

  protected:
    virtual void add_pixel(int2 pixel, float4 const& color, float weight) noexcept = 0;

    virtual void add_pixel_atomic(int2 pixel, float4 const& color, float weight) noexcept = 0;

    virtual void splat_pixel_atomic(int2 pixel, float4 const& color, float weight) noexcept = 0;

    virtual void resolve(int32_t begin, int32_t end, image::Float4& target) const noexcept = 0;

    virtual void resolve_accumulate(int32_t begin, int32_t end, image::Float4& target) const
        noexcept = 0;

    int2 dimensions_;

    float exposure_factor_;
};

}  // namespace rendering::sensor

#endif
