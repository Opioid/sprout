#include "exr_writer.hpp"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "exr.inl"
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

static void reorder(uint8_t* destination, const uint8_t* source, uint32_t len) noexcept {
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

    // Predictor.

    {
        uint8_t* t    = destination + 1;
        uint8_t* stop = destination + len;

        uint32_t p = t[-1];

        for (; t < stop;) {
            uint32_t const d = uint32_t(t[0]) - p + (128 + 256);
            p                = t[0];
            t[0]             = uint8_t(d);
            ++t;
        }
    }
}

bool Writer::zips_compression(std::ostream& stream, Float4 const& image) const noexcept {
    int2 const d = image.description().dimensions2();

    uint32_t const pixel_row_size = uint32_t(d[0] * (4 + 4 + 4));

    mz_stream zip;
    zip.zalloc = nullptr;
    zip.zfree  = nullptr;

    if (MZ_OK != mz_deflateInit(&zip, 6)) {
        return false;
    }

    uint32_t const buffer_size = uint32_t(mz_deflateBound(&zip, pixel_row_size));

    uint8_t* buffer = memory::allocate_aligned<uint8_t>(uint32_t(d[1]) * buffer_size);

    uint8_t* tmp_buffer = memory::allocate_aligned<uint8_t>(pixel_row_size);

    uint8_t* scanline_buffer = memory::allocate_aligned<uint8_t>(pixel_row_size);

    struct Compressed_scanline {
        uint32_t size;
        uint8_t* buffer;
    };

    Compressed_scanline* cs = memory::allocate_aligned<Compressed_scanline>(uint32_t(d[1]));

    uint8_t* current_buffer = buffer;

    float* floats = reinterpret_cast<float*>(scanline_buffer);

    for (int32_t y = 0, i = 0; y < d[1]; ++y) {
        for (int32_t x = 0; x < d[0]; ++x, ++i) {
            float3 const c = image.at(i).xyz();

            floats[d[0] * 0 + x] = 0.f;  // c[2];
            floats[d[0] * 1 + x] = c[1];
            floats[d[0] * 2 + x] = c[0];
        }

        reorder(tmp_buffer, scanline_buffer, pixel_row_size);

        zip.next_in  = tmp_buffer;
        zip.avail_in = pixel_row_size;

        zip.next_out  = current_buffer;
        zip.avail_out = buffer_size;

        if (MZ_STREAM_END != mz_deflate(&zip, MZ_FINISH)) {
            std::cout << "Oh no" << std::endl;
        }

        uint32_t const compressed_size = buffer_size - zip.avail_out;

        std::cout << compressed_size << std::endl;

        cs[y].size   = compressed_size;
        cs[y].buffer = current_buffer;

        current_buffer += compressed_size;

        mz_deflateReset(&zip);
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

    memory::free_aligned(cs);
    memory::free_aligned(buffer);
    memory::free_aligned(tmp_buffer);
    memory::free_aligned(scanline_buffer);

    return true;
}

bool Writer::zip_compression(std::ostream& stream, Float4 const& image) const noexcept {
    int2 const d = image.description().dimensions2();

    int32_t const pixel_row_size = d[0] * (4 + 4 + 4);

    int32_t const block_size = 16 * pixel_row_size;

    mz_stream mzs;
    mzs.zalloc = nullptr;
    mzs.zfree  = nullptr;

    if (MZ_OK != mz_deflateInit(&mzs, 8)) {
        return false;
    }

    int32_t const num_scanline_blocks = d[1] % 16 ? d[1] / 16 + 1 : d[1] / 16;

    uint32_t buffer_size = mz_deflateBound(&mzs, uint32_t(block_size));

    uint8_t* buffer = memory::allocate_aligned<uint8_t>(uint32_t(num_scanline_blocks) *
                                                        buffer_size);

    uint8_t* scanline_block = memory::allocate_aligned<uint8_t>(block_size);

    struct Compressed_scanline_block {
        uint32_t size;
        uint8_t* buffer;
    };

    Compressed_scanline_block* csb = memory::allocate_aligned<Compressed_scanline_block>(
        num_scanline_blocks);

    int32_t block = 0;

    uint8_t* current_buffer = buffer;

    int32_t const floats_per_row = 3 * d[0];

    float* floats = reinterpret_cast<float*>(scanline_block);

    floats[0] = 1.f;
    floats[1] = 0.f;
    floats[2] = 0.f;

    floats[3] = 0.f;
    floats[4] = 0.f;
    floats[5] = 0.f;

    floats[6] = 0.f;
    floats[7] = 0.f;
    floats[8] = 0.f;

    floats[9]  = 0.f;
    floats[10] = 0.f;
    floats[11] = 0.f;

    for (int32_t y = 0, block_row = 0; y < d[1]; ++y) {
        int32_t const o = block_row * floats_per_row;

        for (int32_t x = 0; x < d[0]; ++x) {
            //    float3 const c = image.at(x, y).xyz();

            int32_t a = o + 0 * d[0] + x;
            int32_t b = o + 1 * d[0] + x;
            int32_t c = o + 2 * d[0] + x;

            //       floats[o + 0 * d[0] + x] = 0.f;//c[2];
            //       floats[o + 1 * d[0] + x] = 0.f;//c[1];
            //\       floats[o + 2 * d[0] + x] = 0.f;//c[0];
        }

        //        for (int32_t x = 0; x < d[0]; ++x) {
        //            float3 const c = image.at(x, y).xyz();

        //             floats[0 * 16 * d[0] + block_row * d[0] + x] = 0.f;
        //             floats[4 * 16 * d[0] + block_row * d[0] + x] = 0.f;
        //             floats[8 * 16 * d[0] + block_row * d[0] + x] = 0.f;

        //        }

        if (0 == (y + 1) % 16 || y == d[1] - 1) {
            //    std::cout << y << std::endl;

            mzs.next_in  = scanline_block;
            mzs.avail_in = (block_row + 1) * pixel_row_size;  // block_size;

            mzs.next_out  = current_buffer;
            mzs.avail_out = buffer_size;

            if (MZ_STREAM_END != mz_deflate(&mzs, MZ_FINISH)) {
                std::cout << "Oh no" << std::endl;
            }

            block_row = 0;

            uint32_t const compressed_size = buffer_size - mzs.avail_out;

            std::cout << compressed_size << std::endl;

            csb[block].size   = compressed_size;
            csb[block].buffer = current_buffer;

            ++block;

            current_buffer += compressed_size;

            //   std::cout << uint64_t(current_buffer) << std::endl;

            mz_deflateReset(&mzs);
        } else {
            ++block_row;
        }
    }

    mz_deflateEnd(&mzs);

    int64_t scanline_offset = stream.tellp() + int64_t(num_scanline_blocks * 8);

    for (int32_t y = 0; y < num_scanline_blocks; ++y) {
        //   std::cout << scanline_offset << std::endl;

        w(stream, scanline_offset);

        scanline_offset += 4 + 4 + csb[y].size;
    }

    for (int32_t y = 0; y < num_scanline_blocks; ++y) {
        Compressed_scanline_block const& b = csb[y];

        int32_t row = y * 16;

        stream.write(reinterpret_cast<char*>(&row), 4);
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
