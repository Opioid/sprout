#include "exr_writer.hpp"
#include <fstream>
#include "base/math/vector4.inl"
#include "exr.inl"
#include "image/image.hpp"

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

    {
        w(stream, "compression");
        w(stream, "compression");
        w(stream, 1u);
        stream.put(0x00);
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

    int64_t const scanline_offset = stream.tellp() + static_cast<int64_t>(d[1] * 8);

    int32_t const pixel_row_size = d[0] * (4 + 4 + 4);

    int32_t const row_size = 4 + 4 + pixel_row_size;

    for (int32_t y = 0; y < d[1]; ++y) {
        w(stream, scanline_offset + y * row_size);
    }

    for (int32_t y = 0; y < d[1]; ++y) {
        w(stream, static_cast<uint32_t>(y));

        w(stream, static_cast<uint32_t>(pixel_row_size));

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

    return false;
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
