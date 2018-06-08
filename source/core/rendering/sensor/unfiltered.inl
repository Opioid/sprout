#ifndef SU_CORE_RENDERING_SENSOR_UNFILTERED_INL
#define SU_CORE_RENDERING_SENSOR_UNFILTERED_INL

#include "sampler/camera_sample.hpp"
#include "unfiltered.hpp"

namespace rendering::sensor {

template <class Base, class Clamp>
Unfiltered<Base, Clamp>::Unfiltered(int2 dimensions, float exposure, const Clamp& clamp)
    : Base(dimensions, exposure), clamp_(clamp) {}

template <class Base, class Clamp>
int32_t Unfiltered<Base, Clamp>::filter_radius_int() const {
    return 0;
}

template <class Base, class Clamp>
int4 Unfiltered<Base, Clamp>::isolated_tile(int4 const& tile) const {
    return tile;
}

template <class Base, class Clamp>
void Unfiltered<Base, Clamp>::add_sample(sampler::Camera_sample const& sample, float4 const& color,
                                         int4 const& /*isolated_bounds*/, int4 const& bounds) {
    Base::add_pixel(bounds.xy() + sample.pixel, clamp_.clamp(color), 1.f);
}

}  // namespace rendering::sensor

#endif
