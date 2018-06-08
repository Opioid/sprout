#ifndef SU_CORE_RENDERING_SENSOR_UNFILTERED_HPP
#define SU_CORE_RENDERING_SENSOR_UNFILTERED_HPP

#include "base/math/vector4.hpp"

namespace sampler {
struct Camera_sample;
}

namespace rendering::sensor {

template <class Base, class Clamp>
class Unfiltered : public Base {
  public:
    Unfiltered(int2 dimensions, float exposure, const Clamp& clamp);

    virtual int32_t filter_radius_int() const override final;

    virtual int4 isolated_tile(int4 const& tile) const override final;

    virtual void add_sample(sampler::Camera_sample const& sample, float4 const& color,
                            int4 const& isolated_bounds, int4 const& bounds) override final;

  private:
    Clamp clamp_;
};

}  // namespace rendering::sensor

#endif
