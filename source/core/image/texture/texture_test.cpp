#include "texture_test.hpp"
#include "base/math/print.inl"
#include "base/math/vector3.inl"
#include "image/image.hpp"
#include "texture_encoding.inl"

#include <iostream>

namespace image::texture::testing {

bool is_valid_normal_map(Image const& image, std::string const& filename) {
    if (Image::Type::Byte2 != image.type()) {
        std::cout << "\"" << filename << "\" is not Byte2" << std::endl;
        return false;
    }

    Byte2 const& typed_image = image.byte2();

    auto const d = typed_image.description().dimensions();
    for (int32_t y = 0; y < d[1]; ++y) {
        for (int32_t x = 0; x < d[0]; ++x) {
            auto value = typed_image.at(x, y);

            if (0 == value[0] + value[1]) {
                std::cout << "\"" << filename << "\" [" << x << ", " << y << "] is zero."
                          << std::endl;
                return false;
            }

            float2 n(encoding::cached_snorm_to_float(value[0]),
                     encoding::cached_snorm_to_float(value[1]));

            if (0.f == n[0] && 0.f == n[1]) {
                std::cout << "\"" << filename << "\" [" << x << ", " << y << "] is [0, 0]."
                          << std::endl;

                std::cout << "rg: " << value << std::endl;

                return false;
            }

            /*
            float l = length(n);

            if (std::abs(l - 1.f) > 0.01f) {
                    std::cout << "\"" << filename << "\" [" << x << ", "
                                      << y << "] is degenerate normal." << std::endl;

                    return false;
            }
            */
        }
    }

    return true;
}

}  // namespace image::texture::testing
