#include "glass_sample.hpp"
#include "base/math/math.hpp"
#include "base/math/vector3.inl"
#include "base/spectrum/discrete.inl"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/collision_coefficients.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/material.inl"
#include "scene/material/material_sample.inl"

#include "base/debug/assert.hpp"
#include "scene/material/material_test.hpp"

namespace scene::material::glass {

static void reflect(float3_p wo, float3_p n, float n_dot_wo, bxdf::Sample& result);

static void refract(float3_p wo, float3_p n, float3_p color, float n_dot_wo, float n_dot_t,
                    float eta, bxdf::Sample& result);

Sample::Sample() {
    properties_.unset(Property::Can_evaluate);
}

bxdf::Result Sample::evaluate(float3_p /*wi*/) const {
    return {float3(0.f), 0.f};
}

void Sample::sample(Sampler& sampler, RNG& rng, bxdf::Sample& result) const {
    if (0.f == abbe_) {
        float const p = sampler.sample_1D(rng);

        sample(ior_, p, result);

        result.wavelength = 0.f;
    } else {
        float3 weight;
        float  wavelength = wavelength_;

        float2 const r = sampler.sample_2D(rng);

        if (0.f == wavelength) {
            float const start = Material::Spectrum::start_wavelength();
            float const end   = Material::Spectrum::end_wavelength();

            wavelength = start + (end - start) * r[1];

            weight = Material::spectrum_at_wavelength(wavelength);
            weight *= 3.f;

            /*
            if (i_C.m_Wavelength == 0.0f) {
                    i_C.m_Wavelength = Spectrum::getStartWavelength() +
            (Spectrum::getEndWavelength() - Spectrum::getStartWavelength())* i_r0; o_Weight =
            Spectrum(0.0f); o_Weight.hack_SetAtWavelength(i_C.m_Wavelength, 1.0f); o_Weight *=
            (float)Spectrum::getNumSamples();
                    //o_Weight *= 1.0f/o_Weight.luminance();
            }
            //float hackOffset = (i_C.m_Wavelength - Spectrum::getStartWavelength()) /
            (Spectrum::getEndWavelength() - Spectrum::getStartWavelength());
            //hackOffset *= m_Abbe * 1.0f/100.0f;
            //IoR += hackOffset;

            //From understanding Reference Wavelengths by Carl Zeiss Vision
            //http://www.opticampus.com/files/memo_on_reference_wavelengths.pdf
            IoR += ((IoR - 1.0f)/m_Abbe) * (523655.0f/(i_C.m_Wavelength*i_C.m_Wavelength)
            - 1.5168f);
            */

        } else {
            weight = float3(1.f);
        }

        float const sqr_wl = wavelength * wavelength;

        float const ior = ior_ + ((ior_ - 1.f) / abbe_) * (523655.f / sqr_wl - 1.5168f);

        sample(ior, r[0], result);

        result.reflection *= weight;
        result.wavelength = wavelength;
    }
}

void Sample::set(float ior, float ior_outside, float abbe, float wavelength) {
    ior_         = ior;
    ior_outside_ = ior_outside;
    abbe_        = abbe;
    wavelength_  = wavelength;
}

void Sample::sample(float ior, float p, bxdf::Sample& result) const {
    float eta_i = ior_outside_;
    float eta_t = ior;

    if (eta_i == eta_t) {
        result.reflection = albedo_;
        result.wi         = -wo_;
        result.pdf        = 1.f;
        result.wavelength = 0.f;
        result.type.clear(bxdf::Type::Specular_transmission);
        return;
    }

    float3 n = layer_.n_;

    if (!same_hemisphere(wo_)) {
        n = -n;

        std::swap(eta_i, eta_t);
    }

    float const n_dot_wo = std::min(std::abs(dot(n, wo_)), 1.f);
    float const eta      = eta_i / eta_t;
    float const sint2    = (eta * eta) * (1.f - n_dot_wo * n_dot_wo);

    float n_dot_t;
    float f;
    if (sint2 >= 1.f) {
        n_dot_t = 0.f;

        f = 1.f;
    } else {
        n_dot_t = std::sqrt(1.f - sint2);

        f = fresnel::dielectric(n_dot_wo, n_dot_t, eta_i, eta_t);
    }

    if (p <= f) {
        reflect(wo_, n, n_dot_wo, result);
    } else {
        refract(wo_, n, albedo_, n_dot_wo, n_dot_t, eta, result);
    }
}

void reflect(float3_p wo, float3_p n, float n_dot_wo, bxdf::Sample& result) {
    result.reflection = float3(1.f);
    result.wi         = normalize(2.f * n_dot_wo * n - wo);
    result.pdf        = 1.f;
    result.type.clear(bxdf::Type::Specular_reflection);

    //    SOFT_ASSERT(testing::check(result, sample.wo_, layer));
}

void refract(float3_p wo, float3_p n, float3_p color, float n_dot_wo, float n_dot_t, float eta,
             bxdf::Sample& result) {
    result.reflection = color;
    result.wi         = normalize((eta * n_dot_wo - n_dot_t) * n - eta * wo);
    result.pdf        = 1.f;
    result.type.clear(bxdf::Type::Specular_transmission);

    //    SOFT_ASSERT(testing::check(result, sample.wo_, layer));
}

}  // namespace scene::material::glass
