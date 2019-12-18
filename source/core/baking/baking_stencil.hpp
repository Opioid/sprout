#ifndef SU_CORE_BAKING_STENCIL_HPP
#define SU_CORE_BAKING_STENCIL_HPP

#include "base/math/vector3.inl"

namespace baking {

class Stencil {
  public:
    static uint3 constexpr Dimensions = uint3(5, 5, 9);

    Stencil(float3 const& p, float3 const& wi, float scale) noexcept;

    static uint32_t constexpr Num_data = Dimensions[0] * Dimensions[1] * Dimensions[2];

    float data_[Num_data];
};

}  // namespace baking

#endif
