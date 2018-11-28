#include "material.hpp"
#include "base/json/json.hpp"
#include "base/math/vector4.inl"
#include "base/spectrum/discrete.inl"
#include "base/spectrum/xyz.hpp"
#include "collision_coefficients.inl"
#include "image/texture/texture_adapter.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.hpp"

//#include "image/typed_image.inl"
//#include "image/encoding/png/png_writer.hpp"
//#include "base/spectrum/rgb.hpp"
//#include "base/encoding/encoding.inl"
//#include <fstream>
//#include "base/math/print.hpp"
//#include <iostream>

namespace scene::material {

Material::Material(Sampler_settings const& sampler_settings, bool two_sided) noexcept
    : sampler_key_(sampler_settings.key()), two_sided_(two_sided) {}

Material::~Material() noexcept {}

void Material::set_mask(Texture_adapter const& mask) noexcept {
    mask_ = mask;
}

void Material::set_parameters(json::Value const& parameters) noexcept {
    for (auto& n : parameters.GetObject()) {
        set_parameter(n.name.GetString(), n.value);
    }
}

void Material::compile(thread::Pool& /*pool*/) noexcept {}

void Material::tick(float /*absolute_time*/, float /*time_slice*/) noexcept {}

float3 Material::evaluate_radiance(float3 const& /*wi*/, float2 /*uv*/, float /*area*/,
                                   Filter /*filter*/, Worker const& /*worker*/) const noexcept {
    return float3(0.f);
}

float3 Material::average_radiance(float /*area*/) const noexcept {
    return float3(0.f);
}

bool Material::has_emission_map() const noexcept {
    return false;
}

Material::Sample_2D Material::radiance_sample(float2 r2) const noexcept {
    return {r2, 1.f};
}

float Material::emission_pdf(float2 /*uv*/, Filter /*filter*/, Worker const& /*worker*/) const
    noexcept {
    return 1.f;
}

float Material::opacity(float2 uv, uint64_t /*time*/, Filter filter, Worker const& worker) const
    noexcept {
    if (mask_.is_valid()) {
        auto& sampler = worker.sampler_2D(sampler_key_, filter);
        return mask_.sample_1(sampler, uv);
    } else {
        return 1.f;
    }
}

float3 Material::thin_absorption(float3 const& /*wo*/, float3 const& /*n*/, float2 uv,
                                 uint64_t time, Filter filter, Worker const& worker) const
    noexcept {
    return float3(opacity(uv, time, filter, worker));
}

float3 Material::emission(math::Ray const& /*ray*/, Transformation const& /*transformation*/,
                          float /*step_size*/, rnd::Generator& /*rng*/, Filter /*filter*/,
                          Worker const& /*worker*/) const noexcept {
    return float3::identity();
}

float3 Material::absorption_coefficient(float2 /*uv*/, Filter /*filter*/,
                                        Worker const& /*worker*/) const noexcept {
    return float3::identity();
}

CC Material::collision_coefficients() const noexcept {
    return {float3::identity(), float3::identity()};
}

CC Material::collision_coefficients(float2 /*uv*/, Filter /*filter*/,
                                    Worker const& /*worker*/) const noexcept {
    return {float3::identity(), float3::identity()};
}

CC Material::collision_coefficients(float3 const& /*uvw*/, Filter /*filter*/,
                                    Worker const& /*worker*/) const noexcept {
    return {float3::identity(), float3::identity()};
}

CM Material::control_medium() const noexcept {
    return CM(0.f);
}

volumetric::Gridtree const* Material::volume_tree() const noexcept {
    return nullptr;
}

float Material::similarity_relation_scale(uint32_t /*depth*/) const noexcept {
    return 1.f;
}

bool Material::is_heterogeneous_volume() const noexcept {
    return false;
}

bool Material::is_textured_volume() const noexcept {
    return false;
}

bool Material::is_scattering_volume() const noexcept {
    return true;
}

void Material::prepare_sampling(Shape const& /*shape*/, uint32_t /*part*/, uint64_t /*time*/,
                                Transformation const& /*transformation*/, float /*area*/,
                                bool /*importance_sampling*/, thread::Pool& /*pool*/) noexcept {}

bool Material::is_animated() const noexcept {
    return false;
}

bool Material::has_tinted_shadow() const noexcept {
    return false;
}

uint32_t Material::sampler_key() const noexcept {
    return sampler_key_;
}

bool Material::is_caustic() const noexcept {
    return false;
}

bool Material::is_masked() const noexcept {
    return mask_.is_valid();
}

bool Material::is_emissive() const noexcept {
    if (has_emission_map()) {
        return true;
    }

    float3 const e = average_radiance(1.f);
    return math::any_greater_zero(e);
}

bool Material::is_two_sided() const noexcept {
    return two_sided_;
}

void Material::set_parameter(std::string_view /*name*/, json::Value const& /*value*/) noexcept {}

float3 Material::rainbow_[Num_bands + 1];

void Material::init_rainbow() noexcept {
    Spectrum::init(380.f, 720.f);

    float3 sum_rgb(0.f);
    for (int32_t i = 0; i < Num_bands; ++i) {
        Spectrum temp(0.f);

        temp.set_bin(i, 1.f);

        float3 const rgb = math::saturate(spectrum::XYZ_to_linear_sRGB_D65(temp.normalized_XYZ()));

        rainbow_[i] = rgb;

        sum_rgb += rgb;
    }

    float3 const n = ((1.f / 3.f) * static_cast<float>(Num_bands)) / sum_rgb;

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
        float const wl = Spectrum::start_wavelength() + static_cast<float>(x) * wl_range;
        byte3 const color =
    encoding::float_to_unorm(spectrum::linear_RGB_to_sRGB(spectrum_at_wavelength(wl, 1.f)));

        for (int32_t y = 0; y < d[1]; ++y) {
            image.store(x, y, color);
        }
    }

    image::encoding::png::Writer::write("rainbow.png", image);
    */
}

float3 Material::spectrum_at_wavelength(float lambda, float value) noexcept {
    float const start = Spectrum::start_wavelength();
    float const end   = Spectrum::end_wavelength();
    float const nb    = static_cast<float>(Num_bands);

    float const u = ((lambda - start) / (end - start)) * nb;

    uint32_t const id = static_cast<uint32_t>(u);

    float const frac = u - static_cast<float>(id);

    if (id >= Num_bands) {
        return rainbow_[Num_bands];
    } else {
        return value * math::lerp(rainbow_[id], rainbow_[id + 1], frac);
    }
}

}  // namespace scene::material
