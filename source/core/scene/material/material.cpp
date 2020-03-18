#include "material.hpp"
#include "base/json/json.hpp"
#include "base/math/vector4.inl"
#include "base/spectrum/discrete.inl"
#include "base/spectrum/xyz.hpp"
#include "collision_coefficients.inl"
#include "fresnel/fresnel.inl"
#include "image/texture/texture_adapter.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.hpp"

namespace scene::material {

char const* Material::identifier() {
    return "Material";
}

Material::Material(Sampler_settings const& sampler_settings, bool two_sided)
    : sampler_key_(sampler_settings.key()),
      properties_(two_sided ? Property::Two_sided : Property::None),
      ior_(1.5f) {}

Material::~Material() = default;

void Material::set_mask(Texture_adapter const& mask) {
    mask_ = mask;
}

void Material::set_ior(float ior) {
    ior_ = ior;
}

void Material::commit(thread::Pool& /*threads*/, Scene const& /*scene*/) {}

void Material::simulate(uint64_t /*start*/, uint64_t /*end*/, uint64_t /*frame_length*/,
                        thread::Pool& /*threads*/, Scene const& /*scene*/) {}

float3 Material::evaluate_radiance(float3 const& /*wi*/, float2 /*uv*/, float /*area*/,
                                   Filter /*filter*/, Worker const& /*worker*/) const {
    return float3(0.f);
}

float3 Material::evaluate_radiance(float3 const& /*wi*/, float3 const& /*uvw*/, float /*volume*/,
                                   Filter /*filter*/, Worker const& /*worker*/) const {
    return float3(0.f);
}

float3 Material::average_radiance(float /*area_or_volume*/, Scene const& /*scene*/) const {
    return float3(0.f);
}

Material::Sample_2D Material::radiance_sample(float2 r2) const {
    return {r2, 1.f};
}

float Material::emission_pdf(float2 /*uv*/, Filter /*filter*/, Worker const& /*worker*/) const {
    return 1.f;
}

Material::Sample_3D Material::radiance_sample(float3 const& r3) const {
    return {r3, 1.f};
}

float Material::emission_pdf(float3 const& /*uvw*/, Filter /*filter*/,
                             Worker const& /*worker*/) const {
    return 1.f;
}

float Material::opacity(float2 uv, uint64_t /*time*/, Filter filter, Worker const& worker) const {
    if (mask_.is_valid()) {
        auto const& sampler = worker.sampler_2D(sampler_key_, filter);
        return mask_.sample_1(worker, sampler, uv);
    }

    return 1.f;
}

float3 Material::thin_absorption(float3 const& /*wi*/, float3 const& /*n*/, float2 uv,
                                 uint64_t time, Filter filter, Worker const& worker) const {
    return float3(1.f - opacity(uv, time, filter, worker));
}

float Material::border(float3 const& wi, float3 const& n) const {
    float const f0 = fresnel::schlick_f0(ior_, 1.f);

    float const n_dot_wi = std::max(dot(n, wi), 0.f);

    float const f = 1.f - fresnel::schlick(n_dot_wi, f0);

    return f;
}

float3 Material::absorption_coefficient(float2 /*uv*/, Filter /*filter*/,
                                        Worker const& /*worker*/) const {
    return float3(0.f);
}

CC Material::collision_coefficients() const {
    return {float3(0.f), float3(0.f)};
}

CC Material::collision_coefficients(float2 /*uv*/, Filter /*filter*/,
                                    Worker const& /*worker*/) const {
    return {float3(0.f), float3(0.f)};
}

CC Material::collision_coefficients(float3 const& /*uvw*/, Filter /*filter*/,
                                    Worker const& /*worker*/) const {
    return {float3(0.f), float3(0.f)};
}

CCE Material::collision_coefficients_emission() const {
    return {{float3(0.f), float3(0.f)}, float3(0.f)};
}

CCE Material::collision_coefficients_emission(float3 const& /*uvw*/, Filter /*filter*/,
                                              Worker const& /*worker*/) const {
    return {{float3(0.f), float3(0.f)}, float3(0.f)};
}

CM Material::control_medium() const {
    return CM(0.f);
}

volumetric::Gridtree const* Material::volume_tree() const {
    return nullptr;
}

float Material::similarity_relation_scale(uint32_t /*depth*/) const {
    return 1.f;
}

void Material::prepare_sampling(Shape const& /*shape*/, uint32_t /*part*/, uint64_t /*time*/,
                                Transformation const& /*transformation*/, float /*extent*/,
                                bool /*importance_sampling*/, thread::Pool& /*threads*/,
                                Scene const& /*scene*/) {}

uint32_t Material::sampler_key() const {
    return sampler_key_;
}

bool Material::is_two_sided() const {
    return properties_.is(Property::Two_sided);
}

bool Material::is_masked() const {
    return mask_.is_valid();
}

bool Material::is_animated() const {
    return properties_.is(Property::Animated);
}

bool Material::is_caustic() const {
    return properties_.is(Property::Caustic);
}

bool Material::has_tinted_shadow() const {
    return properties_.is(Property::Tinted_shadow);
}

bool Material::has_emission_map() const {
    return properties_.is(Property::Emission_map);
}

bool Material::is_emissive(Scene const& scene) const {
    if (properties_.is(Property::Emission_map)) {
        return true;
    }

    float3 const e = average_radiance(1.f, scene);
    return any_greater_zero(e);
}

bool Material::is_scattering_volume() const {
    return properties_.is(Property::Scattering_volume);
}

bool Material::is_textured_volume() const {
    return properties_.is(Property::Textured_volume);
}

bool Material::is_heterogeneous_volume() const {
    return properties_.is(Property::Heterogeneous_volume);
}

float Material::ior() const {
    return ior_;
}

float3 Material::rainbow_[Num_bands + 1];

void Material::init_rainbow() {
    Spectrum::init(400.f, 700.f);

    float3 sum_rgb(0.f);
    for (int32_t i = 0; i < Num_bands; ++i) {
        Spectrum temp(0.f);

        temp.set_bin(i, 1.f);

        float3 const rgb = saturate(spectrum::XYZ_to_linear_sRGB_D65(temp.normalized_XYZ()));

        rainbow_[i] = rgb;

        sum_rgb += rgb;
    }

    float3 const n = ((1.f / 3.f) * float(Num_bands)) / sum_rgb;

    // now we hack-normalize it
    for (uint32_t i = 0; i < Num_bands; ++i) {
        rainbow_[i] = n * rainbow_[i];
    }

    rainbow_[Num_bands] = rainbow_[Num_bands - 1];

    /*
    int2 const d(1024, 256);

    image::Image::Description const description(image::Image::Type::Byte3, d);

    image::Byte3 image(description);

    float const wl_range = (Spectrum::end_wavelength() - Spectrum::start_wavelength()) / d[0];

    for (int32_t x = 0; x < d[0]; ++x) {
        float const wl = Spectrum::start_wavelength() + float(x) * wl_range;
        byte3 const color =
        encoding::float_to_unorm(spectrum::linear_to_gamma_sRGB(spectrum_at_wavelength(wl, 1.f)));

        for (int32_t y = 0; y < d[1]; ++y) {
            image.store(x, y, color);
        }
    }

    image::encoding::png::Writer::write("rainbow.png", image);
    */
}

float3 Material::spectrum_at_wavelength(float lambda, float value) {
    float const start = Spectrum::start_wavelength();
    float const end   = Spectrum::end_wavelength();
    float const nb    = float(Num_bands);

    float const u = ((lambda - start) / (end - start)) * nb;

    uint32_t const id = uint32_t(u);

    float const frac = u - float(id);

    if (id >= Num_bands) {
        return rainbow_[Num_bands];
    }

    return value * lerp(rainbow_[id], rainbow_[id + 1], frac);
}

}  // namespace scene::material
