#include "exr_writer.hpp"
#include "base/math/half.inl"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "image/image.hpp"
#include "miniz/miniz.hpp"

#include <fstream>

// Some code is based on
// https://github.com/openexr/openexr/blob/master/OpenEXR/IlmImf/ImfZip.cpp

namespace image::encoding::exr {

Writer::Writer(bool alpha) : alpha_(alpha) {}

std::string Writer::file_extension() const {
    return "exr";
}

static void w(std::ostream& stream, int16_t i) noexcept;
static void w(std::ostream& stream, uint32_t i) noexcept;
static void w(std::ostream& stream, int32_t i) noexcept;
static void w(std::ostream& stream, float f) noexcept;
static void w(std::ostream& stream, int64_t i) noexcept;
static void w(std::ostream& stream, std::string const& s) noexcept;
static void w(std::ostream& stream, Channel const& c) noexcept;

bool Writer::write(std::ostream& stream, Float4 const& image, thread::Pool& /*pool*/) {
    stream.write(reinterpret_cast<const char*>(Signature), Signature_size);

    uint8_t version[4];

    version[0] = 2;
    version[1] = 0;
    version[2] = 0;
    version[3] = 0;

    stream.write(reinterpret_cast<char*>(version), 4);

    {
        w(stream, "channels");
        w(stream, "chlist");

        uint32_t const num_channels = alpha_ ? 4 : 3;

        uint32_t const size = num_channels * (2 + 4 + 4 + 4 + 4) + 1;
        w(stream, size);

        Channel::Type const type = half_ ? Channel::Type::Half : Channel::Type::Float;

        if (alpha_) {
            w(stream, Channel{"A", type});
        }

        w(stream, Channel{"B", type});
        w(stream, Channel{"G", type});
        w(stream, Channel{"R", type});

        stream.put(0x00);
    }

    Compression compression = Compression::ZIP;

    {
        w(stream, "compression");
        w(stream, "compression");
        w(stream, 1u);
        stream.put(static_cast<char>(compression));
    }

    int2 const d  = image.description().dimensions2();
    int2 const dw = d - 1;

    {
        w(stream, "dataWindow");
        w(stream, "box2i");
        w(stream, 16u);

        w(stream, 0u);
        w(stream, 0u);

        stream.write(reinterpret_cast<const char*>(&dw), 8);
    }

    {
        w(stream, "displayWindow");
        w(stream, "box2i");
        w(stream, 16u);

        w(stream, 0u);
        w(stream, 0u);

        stream.write(reinterpret_cast<const char*>(&dw), 8);
    }

    {
        w(stream, "lineOrder");
        w(stream, "lineOrder");
        w(stream, 1u);
        stream.put(0x00);
    }

    {
        w(stream, "pixelAspectRatio");
        w(stream, "float");
        w(stream, 4u);
        w(stream, 1.f);
    }

    {
        w(stream, "screenWindowCenter");
        w(stream, "v2f");
        w(stream, 8u);
        w(stream, 0.f);
        w(stream, 0.f);
    }

    {
        w(stream, "screenWindowWidth");
        w(stream, "float");
        w(stream, 4u);
        w(stream, 1.f);
    }

    stream.put(0x00);

    if (Compression::No == compression) {
        return no_compression(stream, image);
    } else if (Compression::ZIPS == compression) {
        return zip_compression(stream, image, Compression::ZIPS);
    } else if (Compression::ZIP == compression) {
        return zip_compression(stream, image, Compression::ZIP);
    }

    return false;
}

static inline void write_scanline(std::ostream& stream, Float4 const& image, int32_t y,
                                  uint32_t channel, bool half) noexcept {
    int32_t const width = image.description().dimensions2()[0];

    if (half) {
        for (int32_t x = 0; x < width; ++x) {
            float const s = image.at(x, y)[channel];
            w(stream, float_to_half(s));
        }
    } else {
        for (int32_t x = 0; x < width; ++x) {
            float const s = image.at(x, y)[channel];
            w(stream, s);
        }
    }
}

bool Writer::no_compression(std::ostream& stream, Float4 const& image) const noexcept {
    int2 const d = image.description().dimensions2();

    int64_t const scanline_offset = stream.tellp() + int64_t(d[1] * 8);

    int32_t const num_channels = alpha_ ? 4 : 3;

    int32_t const scalar_size = half_ ? 2 : 4;

    int32_t const bytes_per_row = d[0] * num_channels * scalar_size;

    int32_t const row_size = 4 + 4 + bytes_per_row;

    for (int32_t y = 0; y < d[1]; ++y) {
        w(stream, scanline_offset + y * row_size);
    }

    for (int32_t y = 0; y < d[1]; ++y) {
        w(stream, uint32_t(y));

        w(stream, bytes_per_row);

        if (alpha_) {
            write_scanline(stream, image, y, 3, half_);
        }

        write_scanline(stream, image, y, 2, half_);

        write_scanline(stream, image, y, 1, half_);

        write_scanline(stream, image, y, 0, half_);
    }

    return true;
}

static void reorder(uint8_t* destination, uint8_t const* source, uint32_t len) noexcept {
    // Reorder the pixel data.
    {
        uint8_t* t1 = destination;
        uint8_t* t2 = destination + (len + 1) / 2;

        uint8_t const* stop = source + len;

        for (;;) {
            if (source < stop) {
                *(t1++) = *(source++);
            } else {
                break;
            }

            if (source < stop) {
                *(t2++) = *(source++);
            } else {
                break;
            }
        }
    }

    // Predictor
    {
        uint8_t* t = destination + 1;

        uint8_t const* stop = destination + len;

        uint32_t p = uint32_t(t[-1]);

        for (; t < stop;) {
            uint32_t const d = uint32_t(t[0]) - p + (128 + 256);

            p    = t[0];
            t[0] = uint8_t(d);
            ++t;
        }
    }
}

static inline void put_block_half(uint8_t* destination, Float4 const& image, int32_t num_channels,
                                  int32_t num_rows, int32_t i) noexcept {
    int32_t const width = image.description().dimensions2()[0];

    int16_t* halfs = reinterpret_cast<int16_t*>(destination);

    for (int32_t row = 0; row < num_rows; ++row) {
        int32_t const o = row * width * num_channels;
        for (int32_t x = 0; x < width; ++x, ++i) {
            if (4 == num_channels) {
                float4 const c = image.at(i);

                halfs[o + width * 0 + x] = float_to_half(c[3]);
                halfs[o + width * 1 + x] = float_to_half(c[2]);
                halfs[o + width * 2 + x] = float_to_half(c[1]);
                halfs[o + width * 3 + x] = float_to_half(c[0]);

            } else {
                float3 const c = image.at(i).xyz();

                halfs[o + width * 0 + x] = float_to_half(c[2]);
                halfs[o + width * 1 + x] = float_to_half(c[1]);
                halfs[o + width * 2 + x] = float_to_half(c[0]);
            }
        }
    }
}

static inline void put_block_float(uint8_t* destination, Float4 const& image, int32_t num_channels,
                                   int32_t num_rows, int32_t i) noexcept {
    int32_t const width = image.description().dimensions2()[0];

    float* floats = reinterpret_cast<float*>(destination);

    for (int32_t row = 0; row < num_rows; ++row) {
        int32_t const o = row * width * num_channels;
        for (int32_t x = 0; x < width; ++x, ++i) {
            if (4 == num_channels) {
                float4 const c = image.at(i);

                floats[o + width * 0 + x] = c[3];
                floats[o + width * 1 + x] = c[2];
                floats[o + width * 2 + x] = c[1];
                floats[o + width * 3 + x] = c[0];

            } else {
                float3 const c = image.at(i).xyz();

                floats[o + width * 0 + x] = c[2];
                floats[o + width * 1 + x] = c[1];
                floats[o + width * 2 + x] = c[0];
            }
        }
    }
}

bool Writer::zip_compression(std::ostream& stream, Float4 const& image,
                             Compression compression) const noexcept {
    mz_stream zip;
    zip.zalloc = nullptr;
    zip.zfree  = nullptr;

    if (MZ_OK != mz_deflateInit(&zip, MZ_DEFAULT_COMPRESSION)) {
        return false;
    }

    int2 const d = image.description().dimensions2();

    int32_t const rows_per_block = exr::num_scanlines_per_block(compression);
    int32_t const row_blocks     = exr::num_scanline_blocks(d[1], compression);

    int32_t const num_channels = alpha_ ? 4 : 3;

    int32_t const scalar_size = half_ ? 2 : 4;

    uint32_t const bytes_per_row = uint32_t(d[0] * num_channels * scalar_size);

    uint32_t const bytes_per_block = bytes_per_row * uint32_t(rows_per_block);

    memory::Buffer<uint8_t> image_buffer(uint32_t(d[1]) * bytes_per_row);

    memory::Buffer<uint8_t> tmp_buffer(bytes_per_block);

    memory::Buffer<uint8_t> block_buffer(bytes_per_block);

    struct Compressed_block {
        uint32_t size;
        uint8_t* buffer;
    };

    memory::Buffer<Compressed_block> cb(static_cast<uint32_t>(row_blocks));

    uint8_t* current_buffer = image_buffer.data();

    for (int32_t y = 0, i = 0; y < row_blocks; ++y) {
        int32_t const num_rows_here = std::min(d[1] - (y * rows_per_block), rows_per_block);

        if (half_) {
            put_block_half(block_buffer, image, num_channels, num_rows_here, i);
        } else {
            put_block_float(block_buffer, image, num_channels, num_rows_here, i);
        }

        i += num_rows_here * d[0];

        uint32_t const bytes_here = uint32_t(num_rows_here) * bytes_per_row;

        reorder(tmp_buffer, block_buffer, bytes_here);

        zip.next_in  = tmp_buffer;
        zip.avail_in = bytes_here;

        zip.next_out  = current_buffer;
        zip.avail_out = bytes_here;

        mz_deflate(&zip, MZ_FINISH);
        mz_deflateReset(&zip);

        uint32_t const compressed_size = bytes_here - zip.avail_out;

        if (compressed_size >= bytes_here) {
            cb[y].size   = bytes_here;
            cb[y].buffer = current_buffer;

            std::copy(block_buffer.data(), block_buffer + bytes_here, current_buffer);

            current_buffer += bytes_here;
        } else {
            cb[y].size   = compressed_size;
            cb[y].buffer = current_buffer;

            current_buffer += compressed_size;
        }
    }

    mz_deflateEnd(&zip);

    int64_t scanline_offset = stream.tellp() + int64_t(row_blocks * 8);

    for (int32_t y = 0; y < row_blocks; ++y) {
        w(stream, scanline_offset);

        scanline_offset += 4 + 4 + cb[y].size;
    }

    for (int32_t y = 0; y < row_blocks; ++y) {
        Compressed_block const& b = cb[y];

        int32_t const row = y * rows_per_block;
        stream.write(reinterpret_cast<const char*>(&row), 4);
        stream.write(reinterpret_cast<const char*>(&b.size), 4);
        stream.write(reinterpret_cast<char*>(b.buffer), b.size);
    }

    return true;
}

static void w(std::ostream& stream, int16_t i) noexcept {
    stream.write(reinterpret_cast<const char*>(&i), 2);
}

static void w(std::ostream& stream, uint32_t i) noexcept {
    stream.write(reinterpret_cast<const char*>(&i), 4);
}

static void w(std::ostream& stream, int32_t i) noexcept {
    stream.write(reinterpret_cast<const char*>(&i), 4);
}

static void w(std::ostream& stream, float f) noexcept {
    stream.write(reinterpret_cast<const char*>(&f), 4);
}

static void w(std::ostream& stream, int64_t i) noexcept {
    stream.write(reinterpret_cast<const char*>(&i), 8);
}

static void w(std::ostream& stream, std::string const& s) noexcept {
    stream << s;
    stream.put(0x00);
}

static void w(std::ostream& stream, Channel const& c) noexcept {
    w(stream, c.name);

    stream.write(reinterpret_cast<const char*>(&c.type), 4);

    w(stream, 0u);

    uint32_t const sampling = 1;
    w(stream, sampling);
    w(stream, sampling);
}

}  // namespace image::encoding::exr
