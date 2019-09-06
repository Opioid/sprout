#include "exr.hpp"

namespace image::encoding::exr {

int32_t num_scanlines_per_block(Compression compression) noexcept {
    switch (compression) {
        case Compression::No:
        case Compression::RLE:
        case Compression::ZIPS:
            return 1;
        case Compression::ZIP:
        case Compression::PXR24:
            return 16;
        case Compression::PIZ:
        case Compression::B44:
        case Compression::B44A:
            return 32;
    }

    return 1;
}

int32_t num_scanline_blocks(int32_t num_scanlines, Compression compression) noexcept {
    int32_t const pb = num_scanlines_per_block(compression);

    int32_t const x = num_scanlines / pb;

    return num_scanlines % pb ? x + 1 : x;
}

}  // namespace image::encoding::exr
