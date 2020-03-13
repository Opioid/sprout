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

    Unfiltered(Clamp const& clamp);

    int32_t filter_radius_int() const final;

    int4 isolated_tile(int4 const& tile) const final;

    void add_sample(Sample const& sample, float4 const& color, int4 const& isolated, int2 offset,
                    int4 const& bounds) final;

    void splat_sample(Sample_to const& sample, float4 const& color, int2 offset,
                      int4 const& bounds) final;

  private:
    Clamp clamp_;
};

}  // namespace rendering::sensor

#endif
