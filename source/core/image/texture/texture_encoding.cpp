#include "texture_encoding.hpp"
#include "base/encoding/encoding.inl"
#include "base/spectrum/rgb.hpp"

#include <fstream>

namespace image::texture::encoding {

void create_tables() {
    std::ofstream stream("../source/core/image/texture/texture_encoding_tables.inl");

    stream << "#include \"base/memory/const.hpp\"\n\n";

    stream.precision(8);
    stream << std::fixed;

    static uint32_t constexpr Num_samples = 256;

    stream << "SU_GLOBALCONST(float) SRGB_FLOAT[" << Num_samples << "] = {\n";

    stream << "    ";

    for (uint32_t i = 0; i < Num_samples; ++i) {
        stream << spectrum::gamma_to_linear_sRGB(float(i) * (1.f / 255.f)) << "f";

        if (i < Num_samples - 1) {
            stream << ", ";

            if (i > 0 && 0 == ((i + 1) % 8)) {
                stream << "\n    ";
            }
        }
    }

    stream << "};\n\n";

    stream << "SU_GLOBALCONST(float) SNORM_FLOAT[" << Num_samples << "] = {\n";

    stream << "    ";

    for (uint32_t i = 0; i < Num_samples; ++i) {
        stream << ::encoding::snorm_to_float(uint8_t(i)) << "f";

        if (i < Num_samples - 1) {
            stream << ", ";

            if (i > 0 && 0 == ((i + 1) % 8)) {
                stream << "\n    ";
            }
        }
    }

    stream << "};\n\n";

    stream << "SU_GLOBALCONST(float) UNORM_FLOAT[" << Num_samples << "] = {\n";

    stream << "    ";

    for (uint32_t i = 0; i < Num_samples; ++i) {
        stream << ::encoding::unorm_to_float(uint8_t(i)) << "f";

        if (i < Num_samples - 1) {
            stream << ", ";

            if (i > 0 && 0 == ((i + 1) % 8)) {
                stream << "\n    ";
            }
        }
    }

    stream << "};\n";
}

}  // namespace image::texture::encoding
