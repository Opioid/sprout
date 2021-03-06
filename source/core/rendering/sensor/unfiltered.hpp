#ifndef SU_CORE_RENDERING_SENSOR_UNFILTERED_HPP
#define SU_CORE_RENDERING_SENSOR_UNFILTERED_HPP

#include "base/math/vector.hpp"

namespace sampler {
struct Camera_sample;
struct Camera_sample_to;
}  // namespace sampler

namespace rendering::sensor {

namespace aov {
class Value;
}

template <class Base, class Clamp>
class Unfiltered final : public Base {
  public:
    using Sample    = sampler::Camera_sample;
    using Sample_to = sampler::Camera_sample_to;

    Unfiltered(Clamp const& clamp);

    void add_sample(Sample const& sample, float4_p color, aov::Value const* aov, int4_p isolated,
                    int2 offset, int4_p bounds) final;

    void splat_sample(Sample_to const& sample, float4_p color, int2 offset, int4_p bounds) final;

  private:
    Clamp clamp_;
};

}  // namespace rendering::sensor

#endif
