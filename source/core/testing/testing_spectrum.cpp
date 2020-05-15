#include "testing_spectrum.hpp"
#include "base/chrono/chrono.hpp"
#include "base/encoding/encoding.inl"
#include "base/math/print.hpp"
#include "base/math/vector3.inl"
#include "base/random/generator.inl"
#include "base/spectrum/discrete.inl"
#include "base/spectrum/mapping.hpp"
#include "base/spectrum/rgb.hpp"
#include "core/image/encoding/png/png_writer.hpp"
#include "core/image/typed_image.hpp"

#include <iostream>

namespace testing {

static int32_t constexpr Num_bands = 64;

using Spectrum = spectrum::Discrete_spectral_power_distribution<Num_bands>;

void spectrum() {
    std::cout << "testing::spectrum()" << std::endl;

    rnd::Generator rng(0, 0);

    Spectrum::init();

    size_t    num     = 1024 << 12;
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
            xyz = normalize(xyz);
        }

        auto const duration = chrono::seconds_since(start);

        std::cout << xyz << " in " << duration << " s" << std::endl;
    }

    delete[] spectra;
}

void blackbody() {
    static uint32_t constexpr Width  = 1024;
    static uint32_t constexpr Height = 256;

    static float constexpr Min = 800.f;
    static float constexpr Max = 12000.f;

    image::Byte3 image(image::Description(int2(Width, Height)));

    for (uint32_t i = 0; i < Width; ++i) {
        float const t = float(i) / float(Width) * (Max - Min) + Min;

        float3 const color = spectrum::blackbody(t);

        float3 const gamma = spectrum::linear_to_gamma_sRGB(color);

        byte3 const byte = ::encoding::float_to_unorm(gamma);

        for (uint32_t j = 0; j < Height; ++j) {
            image.store(i, j, byte);
        }
    }

    image::encoding::png::Writer::write("blackbody.png", image);
}

}  // namespace testing
