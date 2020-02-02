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

void Sample_dispersion::sample(sampler::Sampler& sampler, bxdf::Sample& result) const {
    float3 weight;
    float  wavelength = wavelength_;

    float2 const r = sampler.generate_sample_2D();

    if (0.f == wavelength) {
        float const start = Material::Spectrum::start_wavelength();
        float const end   = Material::Spectrum::end_wavelength();

        wavelength = start + (end - start) * r[1];

        weight = Material::spectrum_at_wavelength(wavelength);
        weight *= 3.f;

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

    float const sqr_wl = wavelength * wavelength;

    float const ior = ior_ + ((ior_ - 1.f) / abbe_) * (523655.f / sqr_wl - 1.5168f);

    Sample::sample(ior, r[0], result);

    result.reflection *= weight;
    result.wavelength = wavelength;
}

void Sample_dispersion::set_dispersion(float abbe, float wavelength) {
    abbe_       = abbe;
    wavelength_ = wavelength;
}

}  // namespace scene::material::glass
