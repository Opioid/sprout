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
    Unfiltered(int2 dimensions, float exposure, Clamp const& clamp) noexcept;

    int32_t filter_radius_int() const noexcept override final;

    int4 isolated_tile(int4 const& tile) const noexcept override final;

    void add_sample(sampler::Camera_sample const& sample, float4 const& color, int4 const& isolated,
                    int4 const& bounds) noexcept override final;

    void add_sample(sampler::Camera_sample_to const& sample, float4 const& color,
                    int4 const& bounds) noexcept override final;

  private:
    Clamp clamp_;
};

}  // namespace rendering::sensor

#endif
