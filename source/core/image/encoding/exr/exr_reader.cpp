#include "exr_reader.hpp"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "exr.hpp"
#include "image/image.hpp"
#include "logging/logging.hpp"
#include "miniz/miniz.hpp"

#include <cstdint>
#include <cstring>
#include <fstream>

#include <iostream>
#include "base/math/print.hpp"

namespace image::encoding::exr {

static void        channel_list(std::istream &stream) noexcept;
static Compression compression(std::istream &stream) noexcept;
static void        chromaticities(std::istream &stream) noexcept;

static Image *read_zip(std::istream &stream, int2 dimensions) noexcept;

Image *Reader::read(std::istream &stream) noexcept {
    uint8_t header[Signature_size];

    // Check signature
    stream.read(reinterpret_cast<char *>(header), Signature_size);

    if (memcmp(reinterpret_cast<const void *>(Signature), reinterpret_cast<void *>(header),
               Signature_size)) {
        logging::push_error("Bad EXR signature");
        return nullptr;
    }

    // Check version
    stream.read(reinterpret_cast<char *>(header), Signature_size);

    std::cout << uint32_t(header[0]) << " " << uint32_t(header[1]) << " " << uint32_t(header[2])
              << " " << uint32_t(header[3]) << std::endl;

    std::string attribute_name;
    std::string attribute_type;

    Compression compression = Compression::Undefined;

    int4 data_window;
    int4 display_window;

    for (;;) {
        std::getline(stream, attribute_name, '\0');

        if (attribute_name.empty()) {
            break;
        }

        std::cout << attribute_name << ": ";

        if ("channels" == attribute_name) {
        }

        std::getline(stream, attribute_type, '\0');

        std::cout << attribute_type << ": ";

        int32_t attribute_size;
        stream.read(reinterpret_cast<char *>(&attribute_size), sizeof(int32_t));

        std::cout << attribute_size << ": ";

        if ("box2i" == attribute_type) {
            int4 box;
            stream.read(reinterpret_cast<char *>(&box), sizeof(int4));
            std::cout << box;

            if ("dataWindow" == attribute_name) {
                data_window = box;
            } else if ("displayWindow" == attribute_name) {
                display_window = box;
            }

        } else if ("chlist" == attribute_type) {
            channel_list(stream);
        } else if ("compression" == attribute_type) {
            compression = exr::compression(stream);
        } else if ("chromaticities" == attribute_type) {
            chromaticities(stream);
        } else {
            stream.seekg(attribute_size, std::ios_base::cur);
        }

        std::cout << std::endl;
    }

    if (data_window != display_window) {
        logging::push_error("dataWindow != displayWindow");
        return nullptr;
    }

    if (Compression::ZIP == compression) {
        return read_zip(stream, data_window.zw() + 1);
    }

    return nullptr;
}

static Compression compression(std::istream &stream) noexcept {
    Compression type;
    stream.read(reinterpret_cast<char *>(&type), sizeof(uint8_t));

    switch (type) {
        case Compression::No:
            std::cout << "NO_COMPRESSION";
            break;

        case Compression::RLE:
            std::cout << "RLE_COMPRESSION";
            break;

        case Compression::ZIPS:
            std::cout << "ZIPS_COMPRESSION";
            break;

        case Compression::ZIP:
            std::cout << "ZIP_COMPRESSION";
            break;

        case Compression::PIZ:
            std::cout << "PIZ_COMPRESSION";
            break;

        case Compression::PXR24:
            std::cout << "PXR24_COMPRESSION";
            break;

        case Compression::B44:
            std::cout << "B44_COMPRESSION";
            break;

        case Compression::B44A:
            std::cout << "B44A_COMPRESSION";
            break;
        default:
            break;
    }

    return type;
}

static void chromaticities(std::istream &stream) noexcept {
    float c[8];
    stream.read(reinterpret_cast<char *>(c), sizeof(float) * 8);

    std::cout << "[ ";

    for (uint32_t i = 0; i < 8; ++i) {
        std::cout << c[i];

        if (i < 7) {
            std::cout << ", ";
        }
    }

    std::cout << " ]";
}

static void channel_list(std::istream &stream) noexcept {
    std::cout << "[ ";

    for (int32_t i = 0;; ++i) {
        std::string name;
        std::getline(stream, name, '\0');

        if (name.empty()) {
            break;
        } else if (i > 0) {
            std::cout << ", ";
        }

        int32_t type;
        stream.read(reinterpret_cast<char *>(&type), sizeof(int32_t));

        // pLinear
        stream.seekg(1, std::ios_base::cur);

        // reserved
        stream.seekg(3, std::ios_base::cur);

        // xSampling ySampling
        stream.seekg(4 + 4, std::ios_base::cur);

        std::cout << name << " ";

        if (0 == type) {
            std::cout << "uint";
        } else if (1 == type) {
            std::cout << "half";
        } else if (2 == type) {
            std::cout << "float";
        }
    }

    std::cout << " ]";
}

static void reconstruct_scalar(uint8_t *buf, int32_t len) {
    uint8_t *t    = buf + 1;
    uint8_t *stop = buf + len;

    while (t < stop) {
        uint32_t const d = uint32_t(t[-1]) + uint32_t(t[0]) - 128;

        t[0] = uint8_t(d);
        ++t;
    }
}

static void interleave_scalar(uint8_t const *source, int32_t len, uint8_t *out) {
    uint8_t const *t1 = source;
    uint8_t const *t2 = source + (len + 1) / 2;

    uint8_t *      s    = out;
    uint8_t *const stop = s + len;

    while (true) {
        if (s < stop) {
            *(s++) = *(t1++);
        } else {
            break;
        }

        if (s < stop) {
            *(s++) = *(t2++);
        } else {
            break;
        }
    }
}

static void interleave_sse2(uint8_t const *source, int32_t len, uint8_t *out) {
    static int32_t constexpr bytesPerChunk = 2 * sizeof(__m128i);

    int32_t const vOutSize = len / bytesPerChunk;

    __m128i const *v1 = reinterpret_cast<__m128i const *>(source);
    __m128i const *v2 = reinterpret_cast<__m128i const *>(source + (len + 1) / 2);

    __m128i *vOut = reinterpret_cast<__m128i *>(out);

    for (int32_t i = 0; i < vOutSize; ++i) {
        __m128i const a = _mm_loadu_si128(v1++);
        __m128i const b = _mm_loadu_si128(v2++);

        __m128i const lo = _mm_unpacklo_epi8(a, b);
        __m128i const hi = _mm_unpackhi_epi8(a, b);

        _mm_storeu_si128(vOut++, lo);
        _mm_storeu_si128(vOut++, hi);
    }

    uint8_t const *t1 = reinterpret_cast<uint8_t const *>(v1);
    uint8_t const *t2 = reinterpret_cast<uint8_t const *>(v2);

    uint8_t *sOut = reinterpret_cast<uint8_t *>(vOut);

    for (int32_t i = vOutSize * bytesPerChunk; i < len; ++i) {
        *(sOut++) = (i % 2 == 0) ? *(t1++) : *(t2++);
    }
}

static Image *read_zip(std::istream &stream, int2 dimensions) noexcept {
    int32_t const rows_per_block = exr::num_scanlines_per_block(Compression::ZIP);
    int32_t const row_blocks     = exr::num_scanline_blocks(dimensions[1], Compression::ZIP);

    int32_t const bytes_per_row = dimensions[0] * 3 * 4;

    int32_t const bytes_per_row_block = bytes_per_row * rows_per_block;

    auto image = new Image(Float3(Description(dimensions)));

    auto &image_f3 = image->float3();

    for (int32_t i = 0; i < row_blocks; ++i) {
        int64_t v;
        stream.read(reinterpret_cast<char *>(&v), sizeof(int64_t));
    }

    memory::Buffer<uint8_t> compressed(static_cast<uint64_t>(bytes_per_row_block));

    memory::Buffer<uint8_t> uncomressed(static_cast<uint64_t>(bytes_per_row_block));

    memory::Buffer<uint8_t> outdata(static_cast<uint64_t>(bytes_per_row_block));

    for (int32_t i = 0, p = 0; i < row_blocks; ++i) {
        int32_t row;
        stream.read(reinterpret_cast<char *>(&row), sizeof(int32_t));

        int32_t size;
        stream.read(reinterpret_cast<char *>(&size), sizeof(int32_t));

        stream.read(reinterpret_cast<char *>(compressed.data()), size);

        int32_t const num_rows_here = std::min(dimensions[1] - row, rows_per_block);

        int32_t const num_pixels_here = num_rows_here * dimensions[0];

        float *floats;

        if (size < bytes_per_row_block) {
            unsigned long uncompressed_size = uint32_t(bytes_per_row_block);
            if (MZ_OK !=
                mz_uncompress(uncomressed, &uncompressed_size, compressed, uint32_t(size))) {
                return nullptr;
            }

            reconstruct_scalar(uncomressed, num_pixels_here * 3 * 4);

            interleave_sse2(uncomressed, num_pixels_here * 3 * 4, outdata.data());

            floats = reinterpret_cast<float *>(outdata.data());
        } else {
            floats = reinterpret_cast<float *>(compressed.data());
        }

        for (int32_t y = 0; y < num_rows_here; ++y) {
            int32_t const o = 3 * y * dimensions[0];
            for (int32_t x = 0; x < dimensions[0]; ++x, ++p) {
                float const r = floats[o + 2 * dimensions[0] + x];
                float const g = floats[o + 1 * dimensions[0] + x];
                float const b = floats[o + 0 * dimensions[0] + x];

                image_f3.store(p, packed_float3(r, g, b));
            }
        }
    }

    return image;
}

}  // namespace image::encoding::exr
