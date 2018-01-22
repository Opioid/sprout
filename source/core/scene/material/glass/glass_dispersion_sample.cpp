#include "glass_dispersion_sample.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_attenuation.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "sampler/sampler.hpp"
#include "base/math/math.hpp"
#include "base/math/vector3.inl"
#include "base/random/generator.inl"
#include "base/spectrum/discrete.inl"

#include "scene/material/material_test.hpp"
#include "base/debug/assert.hpp"

namespace scene::material::glass {

void Sample_dispersion::sample(sampler::Sampler& sampler, bxdf::Sample& result) const {
	float3 weight;
	float wavelength = wavelength_;
	if (0.f == wavelength) {
		const float start = Material::Spectrum::start_wavelength();
		const float end   = Material::Spectrum::end_wavelength();
		wavelength = start + (end - start) * sampler.rng().random_float();

		weight = Material::spectrum_at_wavelength(wavelength);

		/*
		if (i_C.m_Wavelength == 0.0f) {
			i_C.m_Wavelength = Spectrum::getStartWavelength() + (Spectrum::getEndWavelength() - Spectrum::getStartWavelength())* i_r0;
			o_Weight = Spectrum(0.0f);
			o_Weight.hack_SetAtWavelength(i_C.m_Wavelength, 1.0f);
			o_Weight *= (float)Spectrum::getNumSamples();
			//o_Weight *= 1.0f/o_Weight.luminance();
		}
		//float hackOffset = (i_C.m_Wavelength - Spectrum::getStartWavelength()) / (Spectrum::getEndWavelength() - Spectrum::getStartWavelength());
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

	layer_.ior_ = ior;

	const float p = sampler.generate_sample_1D();

	if (p < 0.5f) {
		BSDF::reflect(*this, layer_, sampler, result);
		result.pdf *= 0.5f;
	} else {
		BSDF::refract(*this, layer_, sampler, result);
		result.pdf *= 0.5f;
	}

	result.reflection *= weight;
	result.wavelength = wavelength;
}

void Sample_dispersion::set(float ior, float abbe, float wavelength) {
	ior_ = ior;
	abbe_ = abbe;
	wavelength_ = wavelength;
}

}

