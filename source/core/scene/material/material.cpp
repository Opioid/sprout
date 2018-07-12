#include "material.hpp"
#include "base/json/json.hpp"
#include "base/math/vector4.inl"
#include "base/spectrum/discrete.inl"
#include "base/spectrum/xyz.hpp"
#include "image/texture/texture_adapter.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.hpp"

namespace scene::material {

Material::Material(Sampler_settings const& sampler_settings, bool two_sided)
    : sampler_key_(sampler_settings.key()), two_sided_(two_sided) {}

Material::~Material() {}

void Material::set_mask(Texture_adapter const& mask) {
    mask_ = mask;
}

void Material::set_parameters(json::Value const& parameters) {
    for (auto& n : parameters.GetObject()) {
        set_parameter(n.name.GetString(), n.value);
    }
}

void Material::compile() {}

void Material::tick(float /*absolute_time*/, float /*time_slice*/) {}

float3 Material::evaluate_radiance(f_float3 /*wi*/, float2 /*uv*/, float /*area*/, float /*time*/,
                                   Sampler_filter /*filter*/, Worker const& /*worker*/) const {
    return float3(0.f);
}

float3 Material::average_radiance(float /*area*/) const {
    return float3(0.f);
}

bool Material::has_emission_map() const {
    return false;
}

Material::Sample_2D Material::radiance_sample(float2 r2) const {
    return {r2, 1.f};
}

float Material::emission_pdf(float2 /*uv*/, Sampler_filter /*filter*/,
                             Worker const& /*worker*/) const {
    return 1.f;
}

float Material::opacity(float2 uv, float /*time*/, Sampler_filter filter,
                        Worker const& worker) const {
    if (mask_.is_valid()) {
        auto& sampler = worker.sampler_2D(sampler_key_, filter);
        return mask_.sample_1(sampler, uv);
    } else {
        return 1.f;
    }
}

float3 Material::thin_absorption(f_float3 /*wo*/, f_float3 /*n*/, float2 uv, float time,
                                 Sampler_filter filter, Worker const& worker) const {
    return float3(opacity(uv, time, filter, worker));
}

float3 Material::emission(math::Ray const& /*ray*/, Transformation const& /*transformation*/,
                          float /*step_size*/, rnd::Generator& /*rng*/, Sampler_filter /*filter*/,
                          Worker const& /*worker*/) const {
    return float3::identity();
}

float3 Material::absorption_coefficient(float2 /*uv*/, Sampler_filter /*filter*/,
                                        Worker const& /*worker*/) const {
    return float3::identity();
}

Material::CC Material::collision_coefficients(float2 /*uv*/, Sampler_filter /*filter*/,
                                              Worker const& /*worker*/) const {
    return {float3::identity(), float3::identity()};
}

Material::CC Material::collision_coefficients(f_float3 /*p*/, Sampler_filter /*filter*/,
                                              Worker const& /*worker*/) const {
    return {float3::identity(), float3::identity()};
}

float Material::majorant_mu_t() const {
    return 0.f;
}

volumetric::Gridtree const* Material::volume_tree() const {
    return nullptr;
}

bool Material::is_heterogeneous_volume() const {
    return false;
}

bool Material::is_scattering_volume() const {
    return true;
}

void Material::prepare_sampling(shape::Shape const& /*shape*/, uint32_t /*part*/,
                                Transformation const& /*transformation*/, float /*area*/,
                                bool /*importance_sampling*/, thread::Pool& /*pool*/) {}

bool Material::is_animated() const {
    return false;
}

bool Material::has_tinted_shadow() const {
    return false;
}

uint32_t Material::sampler_key() const {
    return sampler_key_;
}

bool Material::is_pure_specular() const {
    return false;
}

bool Material::is_masked() const {
    return mask_.is_valid();
}

bool Material::is_emissive() const {
    if (has_emission_map()) {
        return true;
    }

    float3 const e = average_radiance(1.f);
    return math::any_greater_zero(e);
}

bool Material::is_two_sided() const {
    return two_sided_;
}

void Material::set_parameter(std::string const& /*name*/, json::Value const& /*value*/) {}

float3 Material::rainbow_[Num_bands];

void Material::init_rainbow() {
    Spectrum::init(380.f, 720.f);

    //	float const start = Spectrum::start_wavelength();
    //	float const end   = Spectrum::end_wavelength();

    float3 sum_rgb(0.f);
    for (int32_t i = 0; i < Num_bands; ++i) {
        //	float const wl = start + (end - start) * ((static_cast<float>(i) + 0.5f) / nb);
        Spectrum temp;
        temp.clear(0.f);
        //	temp.set_at_wavelength(wl, 1.f);
        temp.set_bin(i, 1.f);
        float3 const rgb = spectrum::XYZ_to_linear_RGB(temp.normalized_XYZ());
        rainbow_[i]      = math::saturate(rgb);
        sum_rgb += rgb;
    }

    constexpr float nb = static_cast<float>(Num_bands);

    // now we hack-normalize it
    for (uint32_t i = 0; i < Num_bands; ++i) {
        rainbow_[i][0] *= nb / sum_rgb[0];
        rainbow_[i][1] *= nb / sum_rgb[1];
        rainbow_[i][2] *= nb / sum_rgb[2];
    }
}

float3 Material::spectrum_at_wavelength(float lambda, float value) {
    float const start = Spectrum::start_wavelength();
    float const end   = Spectrum::end_wavelength();
    float const nb    = static_cast<float>(Num_bands);

    uint32_t const idx = static_cast<uint32_t>(((lambda - start) / (end - start)) * nb);
    if (idx >= Num_bands) {
        return float3::identity();
    } else {
        float const weight = value * (1.f / 3.f);
        return weight * rainbow_[idx];
    }
}

}  // namespace scene::material
