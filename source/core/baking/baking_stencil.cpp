#include "baking_stencil.hpp"
#include "base/math/vector3.inl"

namespace baking {

static float3 constexpr Min(-1.f, -1.f, -1.f);
static float3 constexpr Max(1.f, 1.f, 3.f);

static float3 constexpr Step((Max - Min) / float3(Stencil::Dimensions));

static float3 constexpr Begin(Min + 0.5f * Step);

Stencil::Stencil(float3 const& p, float3 const& wi, float scale) noexcept {
    auto [xa, ya] = orthonormal_basis(wi);

    xa *= scale;
    ya *= scale;

    float3 const za = scale * wi;

    for (uint32_t z = 0, i = 0; z < Dimensions[2]; ++z) {
        for (uint32_t y = 0; y < Dimensions[1]; ++y) {
            for (uint32_t x = 0; x < Dimensions[0]; ++x, ++i) {
                float3 const c = Begin + float3(uint3(x, y, z)) * Step;

                float3 const co = (p + c[0] * xa) + (c[1] * ya + c[2] * za);

                bool const outside = (co[0] < -1.f) | (co[0] > 1.f) | (co[1] < -1.f) |
                                     (co[1] > 1.f) | (co[2] < -1.f) | (co[2] > 1.f);

                data_[i] = outside ? 0.f : 1.f;
            }
        }
    }
}

}  // namespace baking
