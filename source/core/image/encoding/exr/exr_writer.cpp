#include "exr_writer.hpp"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "exr.hpp"
#include "image/image.hpp"
#include "miniz/miniz.hpp"

// Some code is based on
// https://github.com/openexr/openexr/blob/master/OpenEXR/IlmImf/ImfZip.cpp

#include <fstream>

#include <iostream>

namespace image::encoding::exr {

Writer::Writer(bool alpha) : alpha_(alpha) {}

std::string Writer::file_extension() const {
    return "exr";
}

static void w(std::ostream& stream, uint32_t i) noexcept;
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

        //        uint32_t const size = 3 * (2 + 4 + 4 + 4 + 4) + 1;
        //        w(stream, size);

        //        w(stream, Channel{"B", Channel::Type::Float});
        //        w(stream, Channel{"G", Channel::Type::Float});
        //        w(stream, Channel{"R", Channel::Type::Float});
        //        stream.put(0x00);

        uint32_t const size = 3 * (2 + 4 + 4 + 4 + 4) + 1;
        w(stream, size);

        w(stream, Channel{"B", Channel::Type::Float});
        w(stream, Channel{"G", Channel::Type::Float});
        w(stream, Channel{"R", Channel::Type::Float});

        stream.put(0x00);
    }

    Compression compression = Compression::ZIPS;

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
        return zips_compression(stream, image);
    } else if (Compression::ZIP == compression) {
        return zip_compression(stream, image);
    }

    return false;
}

bool Writer::no_compression(std::ostream& stream, Float4 const& image) const noexcept {
    int2 const d = image.description().dimensions2();

    int64_t const scanline_offset = stream.tellp() + int64_t(d[1] * 8);

    int32_t const pixel_row_size = d[0] * (4 + 4 + 4);

    int32_t const row_size = 4 + 4 + pixel_row_size;

    for (int32_t y = 0; y < d[1]; ++y) {
        w(stream, scanline_offset + y * row_size);
    }

    for (int32_t y = 0; y < d[1]; ++y) {
        w(stream, uint32_t(y));

        w(stream, uint32_t(pixel_row_size));

        for (int32_t x = 0; x < d[0]; ++x) {
            float const b = image.at(x, y)[2];
            w(stream, b);
        }

        for (int32_t x = 0; x < d[0]; ++x) {
            float const g = image.at(x, y)[1];
            w(stream, g);
        }

        for (int32_t x = 0; x < d[0]; ++x) {
            float const r = image.at(x, y)[0];
            w(stream, r);
        }
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

bool Writer::zips_compression(std::ostream& stream, Float4 const& image) const noexcept {
    mz_stream zip;
    zip.zalloc = nullptr;
    zip.zfree  = nullptr;

    if (MZ_OK != mz_deflateInit(&zip, MZ_DEFAULT_COMPRESSION)) {
        return false;
    }

    int2 const d = image.description().dimensions2();

    uint32_t const bytes_per_row = uint32_t(d[0] * (4 + 4 + 4));

    memory::Buffer<uint8_t> image_buffer(uint32_t(d[1]) * bytes_per_row);

    memory::Buffer<uint8_t> tmp_buffer(bytes_per_row);

    memory::Buffer<uint8_t> row_buffer(bytes_per_row);

    struct Compressed_scanline {
        uint32_t size;
        uint8_t* buffer;
    };

    memory::Buffer<Compressed_scanline> cs(d[1]);

    uint8_t* current_buffer = image_buffer.data();

    float* floats = reinterpret_cast<float*>(row_buffer.data());

    for (int32_t y = 0, i = 0; y < d[1]; ++y) {
        for (int32_t x = 0; x < d[0]; ++x, ++i) {
            float3 const c = image.at(i).xyz();

            floats[d[0] * 0 + x] = c[2];
            floats[d[0] * 1 + x] = c[1];
            floats[d[0] * 2 + x] = c[0];
        }

        reorder(tmp_buffer, row_buffer, bytes_per_row);

        zip.next_in  = tmp_buffer;
        zip.avail_in = bytes_per_row;

        zip.next_out  = current_buffer;
        zip.avail_out = bytes_per_row;

        mz_deflate(&zip, MZ_FINISH);
        mz_deflateReset(&zip);

        uint32_t const compressed_size = bytes_per_row - zip.avail_out;

        if (compressed_size >= bytes_per_row) {
            cs[y].size   = bytes_per_row;
            cs[y].buffer = current_buffer;

            std::copy(row_buffer.data(), row_buffer + bytes_per_row, current_buffer);

            current_buffer += bytes_per_row;
        } else {
            cs[y].size   = compressed_size;
            cs[y].buffer = current_buffer;

            current_buffer += compressed_size;
        }
    }

    mz_deflateEnd(&zip);

    int64_t scanline_offset = stream.tellp() + int64_t(d[1] * 8);

    for (int32_t y = 0; y < d[1]; ++y) {
        w(stream, scanline_offset);

        scanline_offset += 4 + 4 + cs[y].size;
    }

    for (int32_t y = 0; y < d[1]; ++y) {
        Compressed_scanline const& b = cs[y];

        stream.write(reinterpret_cast<char*>(&y), 4);
        stream.write(reinterpret_cast<const char*>(&b.size), 4);
        stream.write(reinterpret_cast<char*>(b.buffer), b.size);
    }

    return true;
}

bool Writer::zip_compression(std::ostream& stream, Float4 const& image) const noexcept {
    mz_stream zip;
    zip.zalloc = nullptr;
    zip.zfree  = nullptr;

    if (MZ_OK != mz_deflateInit(&zip, MZ_DEFAULT_COMPRESSION)) {
        return false;
    }

    int2 const d = image.description().dimensions2();

    int32_t const rows_per_block = exr::num_scanlines_per_block(Compression::ZIP);
    int32_t const row_blocks     = exr::num_scanline_blocks(d[1], Compression::ZIP);

    int32_t const bytes_per_row = d[0] * 3 * 4;

    int32_t const bytes_per_row_block = bytes_per_row * rows_per_block;

    memory::Buffer<uint8_t> image_buffer(uint32_t(d[1] * bytes_per_row));

    memory::Buffer<uint8_t> tmp_buffer(bytes_per_row_block);

    memory::Buffer<uint8_t> block_buffer(bytes_per_row_block);

    struct Compressed_block {
        uint32_t size;
        uint8_t* buffer;
    };

    memory::Buffer<Compressed_block> cb(d[1]);

    uint8_t* current_buffer = image_buffer.data();

    float* floats = reinterpret_cast<float*>(block_buffer.data());

    for (int32_t y = 0, i = 0; y < row_blocks; ++y) {
        int32_t const num_rows_here = std::min(d[1] - y, rows_per_block);


        /*
        for (int32_t x = 0; x < d[0]; ++x, ++i) {
            float3 const c = image.at(i).xyz();

            floats[d[0] * 0 + x] = c[2];
            floats[d[0] * 1 + x] = c[1];
            floats[d[0] * 2 + x] = c[0];
        }

        reorder(tmp_buffer, row_buffer, bytes_per_row);

        zip.next_in  = tmp_buffer;
        zip.avail_in = bytes_per_row;

        zip.next_out  = current_buffer;
        zip.avail_out = bytes_per_row;

        mz_deflate(&zip, MZ_FINISH);
        mz_deflateReset(&zip);

        uint32_t const compressed_size = bytes_per_row - zip.avail_out;

        if (compressed_size >= bytes_per_row) {
            cs[y].size   = bytes_per_row;
            cs[y].buffer = current_buffer;

            std::copy(row_buffer.data(), row_buffer + bytes_per_row, current_buffer);

            current_buffer += bytes_per_row;
        } else {
            cs[y].size   = compressed_size;
            cs[y].buffer = current_buffer;

            current_buffer += compressed_size;
        }
        */
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

static void w(std::ostream& stream, uint32_t i) noexcept {
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
