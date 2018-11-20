#ifndef SU_CORE_IMAGE_ENCODING_RGBE_READER_HPP
#define SU_CORE_IMAGE_ENCODING_RGBE_READER_HPP

#include <cstdint>
#include <iosfwd>
#include <memory>
#include "base/math/vector3.hpp"
#include "image/typed_image_fwd.hpp"

namespace image::encoding::rgbe {

class Reader {
  public:
    static std::shared_ptr<Image> read(std::istream& stream);

  private:
    struct Header {
        uint32_t width;
        uint32_t height;
    };

    static Header read_header(std::istream& stream);

    static void read_pixels_RLE(std::istream& stream, uint32_t scanline_width,
                                uint32_t num_scanlines, Float3& image);

    static void read_pixels(std::istream& stream, uint32_t num_pixels, Float3& image,
                            uint32_t offset);

    using image_float3 = packed_float3;

    static image_float3 rgbe_to_float3(uint8_t rgbe[4]);
};

}  // namespace image::encoding::rgbe

#endif
