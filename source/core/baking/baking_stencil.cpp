#include "baking_stencil.hpp"
#include "base/math/vector3.inl"

namespace baking {

static float3 constexpr Min(-1.f, -1.f, -1.f);
static float3 constexpr Max(1.f, 1.f, 3.f);

static float3 constexpr Step((Max - Min) / float3(Stencil::Dimensions));

static float3 constexpr Begin(Min + 0.5f * Step);

static float integrate(float3 const& begin, float3 const& end) {
    static uint32_t constexpr Resolution = 8;

    float3 const range = end - begin;

    float3 const step = range / float3(Resolution);

    uint32_t num_inside = 0;

    for (uint32_t z = 0; z < Resolution; ++z) {
        for (uint32_t y = 0; y < Resolution; ++y) {
            for (uint32_t x = 0; x < Resolution; ++x) {
                float3 const c = (0.5f + float3(uint3(x, y, z))) * step;

                float3 const co = begin + c;

                bool const outside = (co[0] < -1.f) | (co[0] > 1.f) | (co[1] < -1.f) |
                                     (co[1] > 1.f) | (co[2] < -1.f) | (co[2] > 1.f);

                if (!outside) {
                    ++num_inside;
                }
            }
        }
    }

    return float(num_inside) / float(Resolution * Resolution * Resolution);
}

Stencil::Stencil(float3 const& p, float3 const& wi, float scale) noexcept {
    auto [xa, ya] = orthonormal_basis(wi);

    xa *= scale;
    ya *= scale;

    float3 const za = scale * wi;

    for (uint32_t z = 0, i = 0; z < Dimensions[2]; ++z) {
        for (uint32_t y = 0; y < Dimensions[1]; ++y) {
            for (uint32_t x = 0; x < Dimensions[0]; ++x, ++i) {
                float3 const b = Min + float3(uint3(x, y, z)) * Step;
                float3 const e = Min + float3(uint3(x + 1, y + 1, z + 1)) * Step;

                float3 const bo = (p + b[0] * xa) + (b[1] * ya + b[2] * za);
                float3 const eo = (p + e[0] * xa) + (e[1] * ya + e[2] * za);

                float const result = integrate(bo, eo);

                data_[i] = result;
            }
        }
    }
}

}  // namespace baking
