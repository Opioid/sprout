#ifndef SU_CORE_RENDERING_SENSOR_UNFILTERED_INL
#define SU_CORE_RENDERING_SENSOR_UNFILTERED_INL

#include "base/math/vector4.inl"
#include "sampler/camera_sample.hpp"
#include "unfiltered.hpp"

namespace rendering::sensor {

template <class Base, class Clamp>
Unfiltered<Base, Clamp>::Unfiltered(float exposure, Clamp const& clamp)
    : Base(exposure), clamp_(clamp) {}

template <class Base, class Clamp>
int32_t Unfiltered<Base, Clamp>::filter_radius_int() const {
    return 0;
}

template <class Base, class Clamp>
int4 Unfiltered<Base, Clamp>::isolated_tile(int4 const& tile) const {
    return tile;
}

template <class Base, class Clamp>
void Unfiltered<Base, Clamp>::add_sample(Sample const& sample, float4 const&   color,
                                         int4 const& /*isolated*/, int4 const& bounds) {
    Base::add_pixel(bounds.xy() + sample.pixel, clamp_.clamp(color), 1.f);
}

template <class Base, class Clamp>
void Unfiltered<Base, Clamp>::splat_sample(Sample_to const& sample, float4 const& color,
                                           int4 const& bounds) {
    Base::splat_pixel_atomic(bounds.xy() + sample.pixel, clamp_.clamp(color), 1.f);
}

}  // namespace rendering::sensor

#endif
