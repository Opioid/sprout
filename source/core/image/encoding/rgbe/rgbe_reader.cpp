#include "rgbe_reader.hpp"
#include "base/math/half.inl"
#include "base/math/vector4.inl"
#include "base/memory/buffer.hpp"
#include "base/spectrum/aces.hpp"
#include "image/image.hpp"
#include "logging/logging.hpp"

#include <cmath>
#include <istream>
#include <string>
#include <type_traits>

// http://www.graphics.cornell.edu/~bjw/rgbe

namespace image::encoding::rgbe {

static uint32_t constexpr Invalid = 0xFFFFFFFF;

struct Header {
    uint32_t width;
    uint32_t height;
};

static Header read_header(std::istream& stream);

template <class Image>
static bool read_pixels_RLE(std::istream& stream, uint32_t scanline_width, uint32_t num_scanlines,
                            Image& image);

template <class Image>
static void read_pixels(std::istream& stream, uint32_t num_pixels, Image& image, int32_t offset);

using image_float3 = packed_float3;

static image_float3 rgbe_to_float3(uint8_t rgbe[4]);

Image* Reader::read(std::istream& stream) {
    Header const header = read_header(stream);

    if (Invalid == header.width) {
        return nullptr;
    }

    int2 const dimensions(header.width, header.height);

    static bool constexpr half_precision = true;

    if (half_precision) {
        auto image = new Image(Short3(Description(dimensions)));

        if (!read_pixels_RLE(stream, header.width, header.height, image->short3())) {
            delete image;
            return nullptr;
        }

        return image;
    }

    auto image = new Image(Float3(Description(dimensions)));

    if (!read_pixels_RLE(stream, header.width, header.height, image->float3())) {
        delete image;
        return nullptr;
    }

    return image;
}

Header read_header(std::istream& stream) {
    std::string line;
    std::getline(stream, line);
    if ("#?" != line.substr(0, 2)) {
        logging::push_error("Bad initial token.");
        return {Invalid, Invalid};
    }

    bool format_specifier = false;

    for (;;) {
        std::getline(stream, line);

        if (line.empty() || 0 == line[0]) {
            // blank lines signifies end of meta data header
            break;
        }

        if ("FORMAT=32-bit_rle_rgbe" == line) {
            format_specifier = true;
        }
    }

    if (!format_specifier) {
        logging::push_error("No FORMAT specifier found.");
        return {Invalid, Invalid};
    }

    Header header;

    std::getline(stream, line);

    if (std::sscanf(line.c_str(), "-Y %d +X %d", &header.height, &header.width) < 2) {
        logging::push_error("Missing image size specifier.");
        return {Invalid, Invalid};
    }

    return header;
}

template <class Image>
static bool read_pixels_RLE(std::istream& stream, uint32_t scanline_width, uint32_t num_scanlines,
                            Image& image) {
    if (scanline_width < 8 || scanline_width > 0x7fff) {
        read_pixels(stream, scanline_width * num_scanlines, image, 0);
        return true;
    }

    int32_t offset = 0;

    uint8_t rgbe[4];
    uint8_t buf[2];

    memory::Buffer<uint8_t> scanline_buffer(4 * scanline_width);

    for (; num_scanlines > 0; --num_scanlines) {
        stream.read(reinterpret_cast<char*>(rgbe), sizeof(rgbe));

        if (rgbe[0] != 2 || rgbe[1] != 2 || (rgbe[2] & 0x80) != 0) {
            // this file is not run length encoded

            if constexpr (std::is_same<Image, Short3>::value) {
                ushort3 const color = float_to_half(rgbe_to_float3(rgbe));

                image.store(0, color);

                read_pixels(stream, scanline_width * num_scanlines - 1, image, 1);
            } else {
                image_float3 const color = rgbe_to_float3(rgbe);

                image.store(0, color);

                read_pixels(stream, scanline_width * num_scanlines - 1, image, 1);
            }

            return true;
        }

        if ((uint32_t(rgbe[2]) << 8 | uint32_t(rgbe[3])) != scanline_width) {
            logging::push_error("Wrong scanline width.");
            return false;
        }

        // read each of the four channels for the scanline into the buffer
        for (uint32_t i = 0, index = 0; i < 4; ++i) {
            uint32_t const end = (i + 1) * scanline_width;

            for (; index < end;) {
                stream.read(reinterpret_cast<char*>(buf), sizeof(buf));

                if (buf[0] > 128) {
                    // a run of the same value
                    uint32_t count = uint32_t(buf[0]) - 128;

                    if (count == 0 || count > end - index) {
                        logging::push_error("Bad scanline data.");
                        return false;
                    }

                    for (; count > 0; --count) {
                        scanline_buffer[index++] = buf[1];
                    }
                } else {
                    // a non-run
                    uint32_t count = uint32_t(buf[0]);

                    if (count == 0 || count > end - index) {
                        logging::push_error("Bad scanline data.");
                        return false;
                    }

                    scanline_buffer[index++] = buf[1];

                    if (--count > 0) {
                        stream.read(reinterpret_cast<char*>(&scanline_buffer[index]), count);

                        index += count;
                    }
                }
            }
        }

        // now convert data from buffer into floats
        for (uint32_t i = 0; i < scanline_width; ++i) {
            rgbe[0] = scanline_buffer[i];
            rgbe[1] = scanline_buffer[i + scanline_width];
            rgbe[2] = scanline_buffer[i + 2 * scanline_width];
            rgbe[3] = scanline_buffer[i + 3 * scanline_width];

            packed_float3 const c = rgbe_to_float3(rgbe);

            if constexpr (std::is_same<Image, Short3>::value) {
                image.store(offset++, float_to_half(c));
            } else {
                image.store(offset++, c);
            }
        }
    }

    return true;
}

template <class Image>
void read_pixels(std::istream& stream, uint32_t num_pixels, Image& image, int32_t offset) {
    uint8_t rgbe[4];

    for (; num_pixels > 0; --num_pixels) {
        stream.read(reinterpret_cast<char*>(rgbe), sizeof(rgbe));

        image_float3 const color = rgbe_to_float3(rgbe);

        if constexpr (std::is_same<Image, Short3>::value) {
            image.store(offset++, float_to_half(color));
        } else {
            image.store(offset++, color);
        }
    }
}

// https://cbloomrants.blogspot.com/2020/06/widespread-error-in-radiance-hdr-rgbe.html
image_float3 rgbe_to_float3(uint8_t rgbe[4]) {
    if (rgbe[3] > 0) {
        // nonzero pixel
        float const f = std::ldexp(1.f, int32_t(rgbe[3]) - (128 + 8));

        float3 const srgb((float(rgbe[0]) + 0.5f) * f, (float(rgbe[1]) + 0.5f) * f,
                          (float(rgbe[2]) + 0.5f) * f);

#ifdef SU_ACESCG
        return image_float3(spectrum::sRGB_to_AP1(srgb));
#else
        return image_float3(srgb);
#endif
    }

    return image_float3(0.f);
}

}  // namespace image::encoding::rgbe
