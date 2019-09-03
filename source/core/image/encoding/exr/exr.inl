#ifndef SU_CORE_IMAGE_ENCODING_EXR_INL
#define SU_CORE_IMAGE_ENCODING_EXR_INL

#include <cstdint>
#include <string>

namespace image::encoding::exr {

static uint32_t constexpr Signature_size = 4;

static uint8_t constexpr Signature[Signature_size] = {0x76, 0x2f, 0x31, 0x01};

struct Channel {
    std::string name;

    enum class Type { Uint = 0, Half, Float };

    Type type;
};

enum class Compression : uint8_t {
    No,
    RLE,
    ZIPS,
    ZIP,
    PIZ,
    PXR24,
    B44,
    B44A
};

}  // namespace image::encoding::exr

#endif
