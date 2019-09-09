#ifndef SU_CORE_IMAGE_ENCODING_EXR_HPP
#define SU_CORE_IMAGE_ENCODING_EXR_HPP

#include <cstdint>
#include <string>

namespace image::encoding::exr {

static uint32_t constexpr Signature_size = 4;

static uint8_t constexpr Signature[Signature_size] = {0x76, 0x2f, 0x31, 0x01};

struct Channel {
    int32_t byte_size() const noexcept;

    std::string name;

    enum class Type { Uint = 0, Half, Float };

    Type type;
};

enum class Compression : uint8_t { No, RLE, ZIPS, ZIP, PIZ, PXR24, B44, B44A, Undefined };

int32_t num_scanlines_per_block(Compression compression) noexcept;

int32_t num_scanline_blocks(int32_t num_scanlines, Compression compression) noexcept;

}  // namespace image::encoding::exr

#endif
