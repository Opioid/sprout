#include "glass_dispersion_sample.hpp"
#include "base/math/math.hpp"
#include "base/math/vector3.inl"
#include "base/random/generator.inl"
#include "base/spectrum/discrete.inl"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/collision_coefficients.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.inl"

#include "base/debug/assert.hpp"
#include "scene/material/material_test.hpp"

namespace scene::material::glass {

void Sample_dispersion::sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept {
    float3 weight;
    float  wavelength = wavelength_;

    if (0.f == wavelength) {
        float const start = Material::Spectrum::start_wavelength();
        float const end   = Material::Spectrum::end_wavelength();

        wavelength = start + (end - start) * sampler.rng().random_float();

        weight = Material::spectrum_at_wavelength(wavelength);

        /*
        if (i_C.m_Wavelength == 0.0f) {
                i_C.m_Wavelength = Spectrum::getStartWavelength() + (Spectrum::getEndWavelength() -
        Spectrum::getStartWavelength())* i_r0; o_Weight = Spectrum(0.0f);
                o_Weight.hack_SetAtWavelength(i_C.m_Wavelength, 1.0f);
                o_Weight *= (float)Spectrum::getNumSamples();
                //o_Weight *= 1.0f/o_Weight.luminance();
        }
        //float hackOffset = (i_C.m_Wavelength - Spectrum::getStartWavelength()) /
        (Spectrum::getEndWavelength() - Spectrum::getStartWavelength());
        //hackOffset *= m_Abbe * 1.0f/100.0f;
        //IoR += hackOffset;

        //From understanding Reference Wavelengths by Carl Zeiss Vision
        //http://www.opticampus.com/files/memo_on_reference_wavelengths.pdf
        IoR += ((IoR - 1.0f)/m_Abbe) * (523655.0f/(i_C.m_Wavelength*i_C.m_Wavelength) - 1.5168f);
        */

    } else {
        weight = float3(1.f);
    }

    float ior = ior_;
    ior += ((ior - 1.f) / abbe_) * (523655.f / (wavelength * wavelength) - 1.5168f);

    float3 n     = layer_.n_;
    float  eta_i = 1.f / ior;
    float  eta_t = ior;

    if (!same_hemisphere(wo_)) {
        n     = -n;
        eta_t = eta_i;
        eta_i = ior;
    }

    float const n_dot_wo = std::min(std::abs(math::dot(n, wo_)), 1.f);
    float const sint2    = (eta_i * eta_i) * (1.f - n_dot_wo * n_dot_wo);

    float n_dot_t;
    float f;
    if (sint2 >= 1.f) {
        n_dot_t = 0.f;
        f       = 1.f;
    } else {
        n_dot_t = std::sqrt(1.f - sint2);
        f       = fresnel::dielectric(n_dot_wo, n_dot_t, eta_i, eta_t);
    }

    float const p = sampler.generate_sample_1D();

    if (p < f) {
        BSDF::reflect(wo_, n, n_dot_wo, result);
    } else {
        BSDF::refract(wo_, n, color_, n_dot_wo, n_dot_t, eta_i, result);
    }

    result.reflection *= weight;
    result.wavelength = wavelength;
}

void Sample_dispersion::set_dispersion(float abbe, float wavelength) noexcept {
    abbe_       = abbe;
    wavelength_ = wavelength;
}

}  // namespace scene::material::glass
