#include "sky_model.hpp"
#include "base/math/matrix3x3.inl"
#include "base/spectrum/aces.hpp"
#include "base/spectrum/discrete.inl"
#include "base/spectrum/xyz.hpp"
#include "hosek/ArHosekSkyModel.hpp"

#include "base/debug/assert.hpp"

namespace procedural::sky {

Model::Model() {
    for (auto& s : skymodel_states_) {
        s = nullptr;
    }

    Spectrum::init(380.f, 720.f);
}

Model::~Model() {
    release();
}

void Model::compile() {
    release();

    float const elevation = std::max(dot(sun_direction_, zenith()) * (-0.5f * Pi), 0.f);

    for (int32_t i = 0; i < Num_bands; ++i) {
        float const ga = ground_albedo_.value(i);

        skymodel_states_[i] = arhosekskymodelstate_alloc_init(elevation, turbidity_, ga);
    }
}

float3 Model::sun_direction() const {
    return sun_direction_;
}

void Model::set_sun_direction(float3 const& direction) {
    sun_direction_ = direction;
}

void Model::set_ground_albedo(float3 const& albedo) {
    ground_albedo_ = Spectrum(albedo);
}

void Model::set_turbidity(float turbidity) {
    turbidity_ = turbidity;
}

float3 Model::evaluate_sky(float3 const& wi) const {
    float const wi_dot_z = std::max(wi[1], 0.00001f);
    float const wi_dot_s = std::min(-dot(wi, sun_direction_), 0.99999f);

    float const gamma = std::acos(wi_dot_s);

    float const sqrt_cos_theta = std::sqrt(wi_dot_z);

    Spectrum radiance;
    for (int32_t i = 0; i < Num_bands; ++i) {
        float const wl_center = Spectrum::wavelength_center(i);
        radiance.set_bin(
            i, float(arhosekskymodel_radiance(skymodel_states_[i], wi_dot_z, sqrt_cos_theta, gamma,
                                              wi_dot_s, wl_center)));
    }

#ifdef SU_ACESCG
    return spectrum::XYZ_to_AP1(radiance.XYZ());
#else
    return max(spectrum::XYZ_to_sRGB(radiance.XYZ()), 0.f);
#endif
}

float3 Model::evaluate_sky_and_sun(float3 const& wi) const {
    float const wi_dot_z = std::max(wi[1], 0.00001f);
    float const wi_dot_s = std::min(-dot(wi, sun_direction_), 0.99999f);

    float const theta = std::acos(wi_dot_z);
    float const gamma = std::acos(wi_dot_s);

    float const sin_gamma_squared = 1.f - wi_dot_s * wi_dot_s;

    float const sqrt_cos_theta = std::sqrt(wi_dot_z);

    ArHosekSkyModelSolarTemp temp;
    arhosekskymodel_solar_radiance_temp(&temp, theta, sin_gamma_squared);

    Spectrum radiance;
    for (int32_t i = 0; i < Num_bands; ++i) {
        float const wl_center = Spectrum::wavelength_center(i);
        radiance.set_bin(
            i, float(arhosekskymodel_solar_radiance(skymodel_states_[i], &temp, wi_dot_z,
                                                    sqrt_cos_theta, gamma, wi_dot_s, wl_center)));
    }

#ifdef SU_ACESCG
    return spectrum::XYZ_to_AP1(radiance.XYZ());
#else
    return max(spectrum::XYZ_to_sRGB(radiance.XYZ()), 0.f);
#endif
}

void Model::release() {
    for (auto s : skymodel_states_) {
        arhosekskymodelstate_free(s);
    }
}

}  // namespace procedural::sky
