#pragma once

#include "base/math/vector3.hpp"
#include "base/spectrum/discrete.hpp"

struct ArHosekSkyModelState;

namespace procedural::sky {

class Model {
  public:
    Model();
    ~Model();

    void compile();

    float3 sun_direction() const;

    void set_sun_direction(float3 const& direction);

    void set_ground_albedo(float3 const& albedo);
    void set_turbidity(float turbidity);

    float3 evaluate_sky(float3 const& wi) const;

    float3 evaluate_sky_and_sun(float3 const& wi) const;

    static float constexpr degrees() {
        // 0.255 should match hosek
        return 0.255f;
    }

    static float3 constexpr zenith() {
        return float3(0.f, 1.f, 0.f);
    }

  private:
    void release();

    float3 sun_direction_;

    float3 ground_albedo_;

    float turbidity_;

    static int32_t constexpr Num_bands = 6;

    ArHosekSkyModelState* skymodel_states_[Num_bands];

    using Spectrum = spectrum::Discrete_spectral_power_distribution<Num_bands>;
};

}  // namespace procedural::sky
