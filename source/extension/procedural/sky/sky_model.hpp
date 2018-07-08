#pragma once

#include "base/math/vector3.hpp"
#include "base/spectrum/discrete.hpp"

struct ArHosekSkyModelState;

namespace procedural::sky {

class Model {
  public:
    Model();
    ~Model();

    // Return signals whether there was a change (not success/failure!)
    bool init();

    float3 sun_direction() const;
    void   set_sun_direction(f_float3 direction);

    void set_ground_albedo(f_float3 albedo);
    void set_turbidity(float turbidity);

    float3 evaluate_sky(f_float3 wi) const;

    float3 evaluate_sky_and_sun(f_float3 wi) const;

    static constexpr float3 zenith() {
        return float3(0.f, 1.f, 0.f);
    };

  private:
    void release();

    float3 sun_direction_;
    float3 ground_albedo_;

    float turbidity_;

    static uint32_t constexpr Num_bands = 6;

    ArHosekSkyModelState* skymodel_states_[Num_bands];

    bool dirty_ = true;

    using Spectrum = spectrum::Discrete_spectral_power_distribution<Num_bands>;
};

}  // namespace procedural::sky
