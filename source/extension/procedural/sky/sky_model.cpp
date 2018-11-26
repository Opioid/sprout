#include "sky_model.hpp"
#include "base/math/matrix3x3.inl"
#include "base/spectrum/discrete.inl"
#include "base/spectrum/xyz.hpp"
#include "hosek/ArHosekSkyModel.hpp"

namespace procedural::sky {

Model::Model() noexcept {
    for (uint32_t i = 0; i < Num_bands; ++i) {
        skymodel_states_[i] = nullptr;
    }

    Spectrum::init(380.f, 720.f);
    //	Spectrum::init(410.f, 690.f);
}

Model::~Model() noexcept {
    release();
}

void Model::compile() noexcept {
    release();

    float const elevation = std::max(math::dot(sun_direction_, zenith()) * (-0.5f * math::Pi), 0.f);

    for (int32_t i = 0; i < Num_bands; ++i) {
        float const ga = ground_albedo_.value(i);

        skymodel_states_[i] = arhosekskymodelstate_alloc_init(elevation, turbidity_, ga);
    }
}

float3 Model::sun_direction() const noexcept {
    return sun_direction_;
}

void Model::set_sun_direction(float3 const& direction) noexcept {
    sun_direction_ = direction;
}

void Model::set_ground_albedo(float3 const& albedo) noexcept {
    ground_albedo_ = Spectrum(albedo);
}

void Model::set_turbidity(float turbidity) noexcept {
    turbidity_ = turbidity;
}

float3 Model::evaluate_sky(float3 const& wi) const noexcept {
    float const wi_dot_z = std::max(wi[1], 0.00001f);
    float const wi_dot_s = std::min(-math::dot(wi, sun_direction_), 0.99999f);

    //	float const theta = std::acos(wi_dot_z);
    float const gamma = std::acos(wi_dot_s);

    /*
    float3 radiance;
    for (uint32_t i = 0; i < 3; ++i) {
            radiance.v[i] =
    static_cast<float>(arhosek_tristim_skymodel_radiance(skymodel_states_[i], theta, gamma, i));
    }
    */

    float const sqrt_cos_theta = std::sqrt(wi_dot_z);

    Spectrum radiance;
    for (int32_t i = 0; i < Num_bands; ++i) {
        float const wl_center = Spectrum::wavelength_center(i);
        radiance.set_bin(i, static_cast<float>(arhosekskymodel_radiance(
                                skymodel_states_[i],
                                /*theta,*/ wi_dot_z, sqrt_cos_theta, gamma, wi_dot_s, wl_center)));
    }

    return spectrum::XYZ_to_linear_sRGB_D65(radiance.XYZ());
}

float3 Model::evaluate_sky_and_sun(float3 const& wi) const noexcept {
    float const wi_dot_z = std::max(wi[1], 0.00001f);
    float const wi_dot_s = std::min(-math::dot(wi, sun_direction_), 0.99999f);

    float const theta = std::acos(wi_dot_z);
    float const gamma = std::acos(wi_dot_s);

    float const sin_gamma = std::sqrt(1.f - wi_dot_s * wi_dot_s);

    float const sqrt_cos_theta = std::sqrt(wi_dot_z);

    ArHosekSkyModelSolarTemp temp;
    arhosekskymodel_solar_radiance_temp(&temp, theta, sin_gamma);

    Spectrum radiance;
    for (int32_t i = 0; i < Num_bands; ++i) {
        float const wl_center = Spectrum::wavelength_center(i);
        radiance.set_bin(i, static_cast<float>(arhosekskymodel_solar_radiance(
                                skymodel_states_[i], &temp, wi_dot_z, sqrt_cos_theta, gamma,
                                wi_dot_s, wl_center)));
    }

    return spectrum::XYZ_to_linear_sRGB_D65(radiance.XYZ());
}

void Model::release() noexcept {
    for (uint32_t i = 0; i < Num_bands; ++i) {
        arhosekskymodelstate_free(skymodel_states_[i]);
    }
}

// float3 const Model::zenith_ = float3(0.f, 1.f, 0.f);

}  // namespace procedural::sky
