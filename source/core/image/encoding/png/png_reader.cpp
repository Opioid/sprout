#include "png_reader.hpp"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "base/spectrum/rgb.hpp"
#include "base/string/string.hpp"
#include "image/image.hpp"
#include "image/tiled_image.inl"
#include "logging/logging.hpp"

#include <cstring>
#include <istream>

// based on
// https://github.com/jansol/LuPng

namespace image::encoding::png {

Reader::Chunk::~Chunk() {
    memory::free_aligned(data);
}

void Reader::Chunk::allocate() {
    if (capacity < length) {
        memory::free_aligned(data);

        data = memory::allocate_aligned<uint8_t>(length);

        capacity = length;
    }
}

enum class Color_type {
    Grayscale       = 0,
    Truecolor       = 2,
    Palleted        = 3,
    Grayscale_alpha = 4,
    Truecolor_alpha = 6
};

Reader::Info::Info() {
    stream.zalloc = nullptr;
    stream.zfree  = nullptr;
}

Reader::Info::~Info() {
    memory::free_aligned(buffer);

    if (stream.zfree) {
        mz_inflateEnd(&stream);
    }
}

bool Reader::Info::allocate() {
    uint32_t const row_size = uint32_t(width * num_channels);

    uint32_t const buffer_size = row_size * uint32_t(height);

    uint32_t const num_bytes = buffer_size + 2 * row_size;

    if (capacity < num_bytes) {
        memory::free_aligned(buffer);

        buffer = memory::allocate_aligned<uint8_t>(num_bytes);

        current_row_data  = buffer + buffer_size;
        previous_row_data = current_row_data + row_size;

        capacity = num_bytes;
    }

    if (!stream.zalloc) {
        if (MZ_OK != mz_inflateInit(&stream)) {
            return false;
        }
    } else {
        if (MZ_OK != mz_inflateReset(&stream)) {
            return false;
        }
    }

    return true;
}

using Info = Reader::Info;

using Chunk = Reader::Chunk;

using Filter = Reader::Filter;

static Image* create_image(Info const& info, Channels channels, int32_t num_elements, bool swap_xy,
                           bool invert);

static bool read_chunk(std::istream& stream, Chunk& chunk);

static bool handle_chunk(std::istream& stream, Chunk& chunk, Info& info);

static bool parse_header(Chunk const& chunk, Info& info);

static bool parse_data(Chunk const& chunk, Info& info);

static uint8_t filter(uint8_t byte, Filter filter, Info const& info);

static uint8_t raw(int column, Info const& info);

static uint8_t prior(int column, Info const& info);

static uint8_t average(uint8_t a, uint8_t b);

static uint8_t paeth_predictor(uint8_t a, uint8_t b, uint8_t c);

static uint32_t byteswap(uint32_t v);

static uint32_t constexpr Signature_size = 8;

static uint8_t constexpr Signature[Signature_size] = {0x89, 0x50, 0x4E, 0x47,
                                                      0x0D, 0x0A, 0x1A, 0x0A};

Image* Reader::read(std::istream& stream, Channels channels, int32_t num_elements, bool swap_xy,
                    bool invert) {
    uint8_t signature[Signature_size];

    stream.read(reinterpret_cast<char*>(signature), Signature_size);

    if (0 != std::memcmp(Signature, signature, Signature_size)) {
        logging::push_error("Bad PNG signature");
        return nullptr;
    }

    for (; handle_chunk(stream, chunk_, info_);) {
    }

    return create_image(info_, channels, num_elements, swap_xy, invert);
}

Image* Reader::create_from_buffer(Channels channels, int32_t num_elements, bool swap_xy,
                                  bool invert) const {
    return create_image(info_, channels, num_elements, swap_xy, invert);
}

Image* create_image(Info const& info, Channels channels, int32_t num_elements, bool swap_xy,
                    bool invert) {
    if (0 == info.num_channels || Channels::None == channels) {
        return nullptr;
    }

    int32_t num_channels;

    switch (channels) {
        case Channels::X:
        case Channels::Y:
        case Channels::Z:
        case Channels::W:
        default:
            num_channels = 1;
            break;
        case Channels::XY:
            num_channels = 2;
            break;
        case Channels::XYZ:
            num_channels = 3;
            break;
        case Channels::XYZW:
            num_channels = 4;
    }

    bool const byte_compatible = num_channels == info.num_channels && !swap_xy && !invert;

    num_channels = std::min(num_channels, info.num_channels);

    int2 dimensions;

    if (1 == num_elements) {
        dimensions[0] = info.width;
        dimensions[1] = info.height;
    } else {
        dimensions[0] = info.width;
        dimensions[1] = info.height / num_elements;
    }

    if (1 == num_channels) {
        Byte1 image(Description(dimensions, num_elements));

        if (byte_compatible) {
            std::memcpy(image.data(), info.buffer, info.width * info.height * num_channels);
        } else {
            int32_t c;

            switch (channels) {
                case Channels::X:
                default:
                    c = 0;
                    break;
                case Channels::Y:
                    c = 1;
                    break;
                case Channels::Z:
                    c = 2;
                    break;
                case Channels::W:
                    c = 3;
                    break;
            }

            if (c >= info.num_channels) {
                c = 0;
            }

            for (int32_t i = 0, len = info.width * info.height; i < len; ++i) {
                int32_t const o = i * info.num_channels;

                uint8_t color = info.buffer[o + c];

                if (invert) {
                    color = 255 - color;
                }

                image.store(i, color);
            }
        }

        return new Image(std::move(image));
    }

    if (2 == num_channels) {
        Byte2 image(Description(dimensions, num_elements));

        if (byte_compatible) {
            std::memcpy(image.data(), info.buffer, info.width * info.height * num_channels);
        } else {
            byte2 color(0, 0);

            for (int32_t i = 0, len = info.width * info.height; i < len; ++i) {
                int32_t const o = i * info.num_channels;
                for (int32_t c = 0; c < num_channels; ++c) {
                    color.v[c] = info.buffer[o + c];
                }

                image.store(i, color);
            }
        }

        return new Image(std::move(image));
    }

    if (3 == num_channels) {
        Byte3 image(Description(dimensions, num_elements));

        if (byte_compatible) {
            std::memcpy(image.data(), info.buffer, info.width * info.height * num_channels);
        } else {
            byte3 color(0, 0, 0);

            for (int32_t i = 0, len = info.width * info.height; i < len; ++i) {
                int32_t const o = i * info.num_channels;
                for (int32_t c = 0; c < num_channels; ++c) {
                    color.v[c] = info.buffer[o + c];
                }

                if (swap_xy) {
                    std::swap(color[0], color[1]);
                }

                image.store(i, color);
            }
        }

        return new Image(std::move(image));
    }

    if (4 == num_channels) {
        Byte4 image(Description(dimensions, num_elements));

        if (byte_compatible) {
            std::memcpy(image.data(), info.buffer, info.width * info.height * num_channels);
        } else {
            byte4 color(0, 0, 0, 255);

            for (int32_t i = 0, len = info.width * info.height; i < len; ++i) {
                int32_t const o = i * info.num_channels;
                for (int32_t c = 0; c < num_channels; ++c) {
                    color.v[c] = info.buffer[o + c];
                }

                if (swap_xy) {
                    std::swap(color[0], color[1]);
                }

                image.store(i, color);
            }
        }

        return new Image(std::move(image));
    }

    return nullptr;
}

bool read_chunk(std::istream& stream, Chunk& chunk) {
    chunk.allocate();

    stream.read(reinterpret_cast<char*>(chunk.data), chunk.length);

    // crc
    stream.ignore(4);

    return stream.good();
}

bool handle_chunk(std::istream& stream, Chunk& chunk, Info& info) {
    uint32_t length = 0;
    stream.read(reinterpret_cast<char*>(&length), sizeof(uint32_t));
    length = byteswap(length);

    // Max chunk length according to spec
    if (length > 0x7FFFFFFF) {
        return false;
    }

    chunk.length = length;

    uint32_t type = 0;
    stream.read(reinterpret_cast<char*>(&type), sizeof(uint32_t));

    // IHDR: 0x52444849
    if (0x52444849 == type) {
        return read_chunk(stream, chunk) && parse_header(chunk, info);
    }

    // IDAT: 0x54414449
    if (0x54414449 == type) {
        return read_chunk(stream, chunk) && parse_data(chunk, info);
    }

    // IEND: 0x444E4549
    if (0x444E4549 == type) {
        return false;
    }

    stream.ignore(length + 4);

    return stream.good();
}

static bool header_error(std::string const& text, Info& info) {
    logging::push_error(text);
    info.num_channels = 0;
    return false;
}

bool parse_header(Chunk const& chunk, Info& info) {
    info.width  = int32_t(byteswap(reinterpret_cast<uint32_t*>(chunk.data)[0]));
    info.height = int32_t(byteswap(reinterpret_cast<uint32_t*>(chunk.data)[1]));

    uint32_t const depth = uint32_t(chunk.data[8]);
    if (8 != depth) {
        return header_error(string::to_string(depth) + " PNG bit depth not supported.", info);
    }

    const Color_type color_type = static_cast<Color_type>(chunk.data[9]);

    switch (color_type) {
        case Color_type::Grayscale:
            info.num_channels = 1;
            break;
        case Color_type::Truecolor:
            info.num_channels = 3;
            break;
        case Color_type::Truecolor_alpha:
            info.num_channels = 4;
            break;
        default:
            info.num_channels = 0;
            break;
    }

    if (0 == info.num_channels) {
        return header_error("Indexed PNG image not supported.", info);
    }

    info.bytes_per_pixel = info.num_channels;

    uint8_t const interlace = chunk.data[12];
    if (interlace) {
        return header_error("Interlaced PNG image not supported.", info);
    }

    info.current_filter     = Filter::None;
    info.filter_byte        = true;
    info.current_byte       = 0;
    info.current_byte_total = 0;

    if (!info.allocate()) {
        return header_error("Could not deflate PNG buffers.", info);
    }

    return true;
}

bool parse_data(Chunk const& chunk, Info& info) {
    static uint32_t constexpr Buffer_size = 8192;
    uint8_t buffer[Buffer_size];

    info.stream.next_in  = chunk.data;
    info.stream.avail_in = chunk.length;

    int32_t const row_size = info.width * info.num_channels;

    do {
        info.stream.next_out  = buffer;
        info.stream.avail_out = Buffer_size;

        int const status = mz_inflate(&info.stream, MZ_NO_FLUSH);
        if (status != MZ_OK && status != MZ_STREAM_END && status != MZ_BUF_ERROR &&
            status != MZ_NEED_DICT) {
            return false;
        }

        uint32_t const decompressed = Buffer_size - info.stream.avail_out;

        for (uint32_t i = 0; i < decompressed; ++i) {
            if (info.filter_byte) {
                info.current_filter = Filter(buffer[i]);
                info.filter_byte    = false;
            } else {
                uint8_t const raw = filter(buffer[i], info.current_filter, info);
                info.current_row_data[info.current_byte] = raw;
                info.buffer[info.current_byte_total++]   = raw;

                if (row_size - 1 == info.current_byte) {
                    info.current_byte = 0;
                    std::swap(info.current_row_data, info.previous_row_data);
                    info.filter_byte = true;
                } else {
                    ++info.current_byte;
                }
            }
        }
    } while (info.stream.avail_in > 0 || 0 == info.stream.avail_out);

    return true;
}

uint8_t filter(uint8_t byte, Filter filter, Info const& info) {
    switch (filter) {
        case Filter::None:
            return byte;
        case Filter::Sub:
            return byte + raw(info.current_byte - info.bytes_per_pixel, info);
        case Filter::Up:
            return byte + prior(info.current_byte, info);
        case Filter::Average:
            return byte + average(raw(info.current_byte - info.bytes_per_pixel, info),
                                  prior(info.current_byte, info));
        case Filter::Paeth:
            return byte + paeth_predictor(raw(info.current_byte - info.bytes_per_pixel, info),
                                          prior(info.current_byte, info),
                                          prior(info.current_byte - info.bytes_per_pixel, info));
        default:
            return 0;
    }
}

uint8_t raw(int column, Info const& info) {
    if (column < 0) {
        return 0;
    }

    return info.current_row_data[column];
}

uint8_t prior(int column, Info const& info) {
    if (column < 0) {
        return 0;
    }

    return info.previous_row_data[column];
}

uint8_t average(uint8_t a, uint8_t b) {
    return uint8_t((uint32_t(a) + uint32_t(b)) >> 1);
}

uint8_t paeth_predictor(uint8_t a, uint8_t b, uint8_t c) {
    int32_t const A  = int32_t(a);
    int32_t const B  = int32_t(b);
    int32_t const C  = int32_t(c);
    int32_t const p  = A + B - C;
    int32_t const pa = std::abs(p - A);
    int32_t const pb = std::abs(p - B);
    int32_t const pc = std::abs(p - C);

    if (pa <= pb && pa <= pc) {
        return a;
    }

    if (pb <= pc) {
        return b;
    }

    return c;
}

uint32_t byteswap(uint32_t v) {
    return ((v & 0xFF) << 24) | ((v & 0xFF00) << 8) | ((v & 0xFF0000) >> 8) |
           ((v & 0xFF000000) >> 24);
}

}  // namespace image::encoding::png
