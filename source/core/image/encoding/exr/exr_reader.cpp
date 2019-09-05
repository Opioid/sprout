#include "exr_reader.hpp"
#include "base/math/vector2.inl"
#include "base/memory/align.hpp"
#include "exr.inl"
#include "image/image.hpp"
#include "logging/logging.hpp"
#include "miniz/miniz.hpp"

#include <cstdint>
#include <cstring>
#include <fstream>

#include <iostream>
#include "base/math/print.hpp"

namespace image::encoding::exr {

void channel_list(std::istream &stream) noexcept;
void compression(std::istream &stream) noexcept;
void chromaticities(std::istream &stream) noexcept;

static void reconstruct_scalar(char *buf, size_t outSize) {
    unsigned char *t    = (unsigned char *)buf + 1;
    unsigned char *stop = (unsigned char *)buf + outSize;

    while (t < stop) {
        int d = int(t[-1]) + int(t[0]) - 128;
        t[0]  = d;
        ++t;
    }
}

static void interleave_scalar(const char *source, size_t outSize, char *out) {
    const char *t1   = source;
    const char *t2   = source + (outSize + 1) / 2;
    char *      s    = out;
    char *const stop = s + outSize;

    while (true) {
        if (s < stop)
            *(s++) = *(t1++);
        else
            break;

        if (s < stop)
            *(s++) = *(t2++);
        else
            break;
    }
}

static void interleave_sse2(const char *source, size_t outSize, char *out) {
    static const size_t bytesPerChunk = 2 * sizeof(__m128i);

    const size_t vOutSize = outSize / bytesPerChunk;

    const __m128i *v1   = reinterpret_cast<const __m128i *>(source);
    const __m128i *v2   = reinterpret_cast<const __m128i *>(source + (outSize + 1) / 2);
    __m128i *      vOut = reinterpret_cast<__m128i *>(out);

    for (size_t i = 0; i < vOutSize; ++i) {
        __m128i a = _mm_loadu_si128(v1++);
        __m128i b = _mm_loadu_si128(v2++);

        __m128i lo = _mm_unpacklo_epi8(a, b);
        __m128i hi = _mm_unpackhi_epi8(a, b);

        _mm_storeu_si128(vOut++, lo);
        _mm_storeu_si128(vOut++, hi);
    }

    const char *t1   = reinterpret_cast<const char *>(v1);
    const char *t2   = reinterpret_cast<const char *>(v2);
    char *      sOut = reinterpret_cast<char *>(vOut);

    for (size_t i = vOutSize * bytesPerChunk; i < outSize; ++i) {
        *(sOut++) = (i % 2 == 0) ? *(t1++) : *(t2++);
    }
}

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

    std::string attribute;

    for (;;) {
        std::getline(stream, attribute, '\0');

        if (attribute.empty()) {
            break;
        }

        std::cout << attribute << ": ";

        if ("channels" == attribute) {
        }

        std::getline(stream, attribute, '\0');

        std::cout << attribute << ": ";

        int32_t attribute_size;
        stream.read(reinterpret_cast<char *>(&attribute_size), sizeof(int32_t));

        std::cout << attribute_size << ": ";

        if ("box2i" == attribute) {
            int4 box;
            stream.read(reinterpret_cast<char *>(&box), sizeof(int4));
            std::cout << box;
        } else if ("chlist" == attribute) {
            channel_list(stream);
        } else if ("compression" == attribute) {
            compression(stream);
        } else if ("chromaticities" == attribute) {
            chromaticities(stream);
        } else {
            stream.seekg(attribute_size, std::ios_base::cur);
        }

        std::cout << std::endl;
    }

    std::cout << "scanline offset table:" << std::endl;

    int64_t v;
    stream.read(reinterpret_cast<char *>(&v), sizeof(int64_t));
    std::cout << v << std::endl;

    std::cout << "scanlines:" << std::endl;

    {
        int32_t y;
        stream.read(reinterpret_cast<char *>(&y), sizeof(int32_t));

        int32_t size;
        stream.read(reinterpret_cast<char *>(&size), sizeof(int32_t));

        std::cout << "y: " << y << std::endl;
        std::cout << "size: " << size << std::endl;

        uint8_t *compressed = memory::allocate_aligned<uint8_t>(size);

        stream.read(reinterpret_cast<char *>(compressed), size);

        float *data = memory::allocate_aligned<float>(4 * 3);

        float *outdata = memory::allocate_aligned<float>(4 * 3);

        mz_stream mzs;
        mzs.zalloc = nullptr;
        mzs.zfree  = nullptr;

        if (MZ_OK != mz_inflateInit(&mzs)) {
            return nullptr;
        }

        mzs.next_in  = compressed;
        mzs.avail_in = size;

        mzs.next_out  = reinterpret_cast<uint8_t *>(data);
        mzs.avail_out = 4 * 3 * 4;

        mz_inflate(&mzs, MZ_FINISH);

        mz_inflateEnd(&mzs);

        reconstruct_scalar(reinterpret_cast<char *>(data), 4 * 3 * 4);

        interleave_sse2(reinterpret_cast<char *>(data), 4 * 3 * 4,
                        reinterpret_cast<char *>(outdata));

        for (uint32_t i = 0; i < 12; ++i) {
            std::cout << outdata[i] << std::endl;
        }

        memory::free_aligned(data);
        memory::free_aligned(compressed);
    }

    return nullptr;
}

void compression(std::istream &stream) noexcept {
    uint8_t type;
    stream.read(reinterpret_cast<char *>(&type), sizeof(uint8_t));

    if (0 == type) {
        std::cout << "NO_COMPRESSION";
    } else if (1 == type) {
        std::cout << "RLE_COMPRESSION";
    } else if (2 == type) {
        std::cout << "ZIPS_COMPRESSION";
    } else if (3 == type) {
        std::cout << "ZIP_COMPRESSION";
    } else if (4 == type) {
        std::cout << "PIZ_COMPRESSION";
    } else if (5 == type) {
        std::cout << "PXR24_COMPRESSION";
    } else if (6 == type) {
        std::cout << "B44_COMPRESSION";
    } else if (7 == type) {
        std::cout << "B44A_COMPRESSION";
    }
}

void chromaticities(std::istream &stream) noexcept {
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

void channel_list(std::istream &stream) noexcept {
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

}  // namespace image::encoding::exr
