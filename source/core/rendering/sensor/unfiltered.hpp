#ifndef SU_CORE_RENDERING_SENSOR_UNFILTERED_HPP
#define SU_CORE_RENDERING_SENSOR_UNFILTERED_HPP

#include "base/math/vector.hpp"

namespace sampler {
struct Camera_sample;
struct Camera_sample_to;
}  // namespace sampler

namespace rendering::sensor {

template <class Base, class Clamp>
class Unfiltered final : public Base {
  public:
    using Sample    = sampler::Camera_sample;
    using Sample_to = sampler::Camera_sample_to;

    Unfiltered(float exposure, Clamp const& clamp) noexcept;

    int32_t filter_radius_int() const noexcept final;

    int4 isolated_tile(int4 const& tile) const noexcept final;

    void add_sample(Sample const& sample, float4 const& color, int4 const& isolated,
                    int4 const& bounds) noexcept final;

    void splat_sample(Sample_to const& sample, float4 const& color,
                      int4 const& bounds) noexcept final;

  private:
    Clamp clamp_;
};

}  // namespace rendering::sensor

#endif
