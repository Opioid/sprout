#ifndef SU_EXTENSION_PROCEDURAL_SKY_MODEL_HPP
#define SU_EXTENSION_PROCEDURAL_SKY_MODEL_HPP

#include "base/math/math.hpp"
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

    static float constexpr angular_radius() noexcept {
        // 0.5 * 0.51 degrees should match hosek
        // ... but we changed hosek ...
        // return degrees_to_radians(0.5f * 0.51f);
        return degrees_to_radians(0.5f * 0.5334f);
    }

    static float radius() noexcept {
        return std::tan(angular_radius());
    }

    static float3 constexpr zenith() noexcept {
        return float3(0.f, 1.f, 0.f);
    }

  private:
    void release() noexcept;

    static int32_t constexpr Num_bands = 12;

    using Spectrum = spectrum::Discrete_spectral_power_distribution<Num_bands>;

    float3 sun_direction_;

    Spectrum ground_albedo_;

    float turbidity_;

    ArHosekSkyModelState* skymodel_states_[Num_bands];
};

}  // namespace procedural::sky

#endif
