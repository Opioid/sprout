#include "texture_test.hpp"
#include <iostream>
#include "base/math/print.hpp"
#include "base/math/vector3.inl"
#include "image/typed_image.inl"
#include "texture_encoding.hpp"

namespace image::texture::testing {

bool is_valid_normal_map(const Image& image, std::string const& filename) {
    if (Image::Type::Byte3 != image.description().type) {
        std::cout << "\"" << filename << "\" is not Byte3" << std::endl;
        return false;
    }

    Byte3 const& typed_image = static_cast<Byte3 const&>(image);

    int2 d = image.description().dimensions.xy();
    for (int32_t y = 0; y < d[1]; ++y) {
        for (int32_t x = 0; x < d[0]; ++x) {
            auto value = typed_image.at(x, y);

            if (0 == value[0] + value[1] + value[2]) {
                std::cout << "\"" << filename << "\" [" << x << ", " << y << "] is zero."
                          << std::endl;
                return false;
            }

            float3 n(encoding::cached_snorm_to_float(value[0]),
                     encoding::cached_snorm_to_float(value[1]),
                     encoding::cached_snorm_to_float(value[2]));

            if (0.f == n[0] && 0.f == n[1] && 0.f == n[2]) {
                std::cout << "\"" << filename << "\" [" << x << ", " << y << "] is [0, 0, 0]."
                          << std::endl;

                std::cout << "rgb: " << value << std::endl;

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
