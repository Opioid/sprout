#include "testing_spectrum.hpp"
#include "base/chrono/chrono.hpp"
#include "base/random/generator.inl"
#include "base/spectrum/discrete.inl"
#include "base/math/vector3.inl"
#include "base/math/print.hpp"
#include <iostream>

namespace testing {

static int32_t constexpr Num_bands = 64;

using Spectrum = spectrum::Discrete_spectral_power_distribution<Num_bands>;

void spectrum() {
	std::cout << "testing::spectrum()" << std::endl;

	rnd::Generator rng;

	Spectrum::init();

	size_t num = 1024 << 12;
	Spectrum* spectra = new Spectrum[num];

	for (size_t i = 0; i < num; ++i) {
		for (int32_t b = 0, len = Spectrum::num_bands(); b < len; ++b) {
			spectra[i].set_bin(b, rng.random_float());
		}
	}

	{
		float3 xyz(0.f);

		auto start = std::chrono::high_resolution_clock::now();

		for (size_t i = 0; i < num; ++i) {
			xyz += spectra[i].XYZ();
			xyz = math::normalize(xyz);
		}

		auto const duration = chrono::seconds_since(start);

		std::cout << xyz << " in " << duration << " s" << std::endl;
	}

	delete[] spectra;
}

}
