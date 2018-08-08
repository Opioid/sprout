#pragma once

#include "base/math/vector3.hpp"
#include "base/spectrum/discrete.hpp"

struct ArHosekSkyModelState;

namespace procedural::sky {

class Model {
  public:
    Model() noexcept;

    ~Model() noexcept;

    void compile() noexcept;

    float3 sun_direction() const noexcept;

    void set_sun_direction(float3 const& direction) noexcept;

    void set_ground_albedo(float3 const& albedo) noexcept;

    void set_turbidity(float turbidity) noexcept;

    float3 evaluate_sky(float3 const& wi) const noexcept;

    float3 evaluate_sky_and_sun(float3 const& wi) const noexcept;

    static float constexpr degrees() noexcept {
        // 0.255 should match hosek
        return 0.255f;
    }

    static float3 constexpr zenith() noexcept {
        return float3(0.f, 1.f, 0.f);
    }

  private:
    void release() noexcept;

    float3 sun_direction_;

    float3 ground_albedo_;

    float turbidity_;

    static int32_t constexpr Num_bands = 6;

    ArHosekSkyModelState* skymodel_states_[Num_bands];

    using Spectrum = spectrum::Discrete_spectral_power_distribution<Num_bands>;
};

}  // namespace procedural::sky
