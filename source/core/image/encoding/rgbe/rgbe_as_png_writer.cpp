#include "rgbe_as_png_writer.hpp"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "base/thread/thread_pool.hpp"
#include "image/typed_image.hpp"
#include "miniz/miniz.hpp"

#include <cmath>
#include <ostream>

#include <iostream>

namespace image::encoding::rgbe {

static byte4 float_to_rgbe(float4 const& c);
static byte4 float_to_rgbd(float4 const& c, float max);

std::string Rgbe_as_png::file_extension() const {
    return "rgbe.png";
}

bool Rgbe_as_png::write(std::ostream& stream, Float4 const& image, thread::Pool& threads) {
    int2 const d = image.description().dimensions_2();

    memory::Buffer<byte4> buffer(d[0] * d[1]);

    threads.run_range(
        [&buffer, &image](uint32_t /*id*/, int32_t begin, int32_t end) noexcept {
            for (int32_t i = begin; i < end; ++i) {
                buffer[i] = float_to_rgbe(image.at(i));
            }
        },
        0, d[0] * d[1]);

    int32_t const num_channels = 4;

    size_t buffer_len = 0;
    void*  png_buffer = tdefl_write_image_to_png_file_in_memory(
        reinterpret_cast<char*>(buffer.data()), d[0], d[1], num_channels, &buffer_len);
    if (!png_buffer) {
        return false;
    }

    stream.write(static_cast<char*>(png_buffer), buffer_len);

    mz_free(png_buffer);

    return true;
}

std::string Rgbd_as_png::file_extension() const {
    return "rgbd.png";
}

bool Rgbd_as_png::write(std::ostream& stream, Float4 const& image, thread::Pool& threads) {
    int2 const d = image.description().dimensions_2();

    memory::Buffer<byte4> buffer(d[0] * d[1]);

    float max = 0.f;

    for (int32_t i = 0, len = d[0] * d[1]; i < len; ++i) {
        float const pm = max_component(image.at(i).xyz());

        max = std::max(max, pm);
    }

    std::cout << max << std::endl;

    threads.run_range(
        [&buffer, &image, max](uint32_t /*id*/, int32_t begin, int32_t end) noexcept {
            for (int32_t i = begin; i < end; ++i) {
                buffer[i] = float_to_rgbd(image.at(i), max);
            }
        },
        0, d[0] * d[1]);

    int32_t const num_channels = 4;

    size_t buffer_len = 0;
    void*  png_buffer = tdefl_write_image_to_png_file_in_memory(
        reinterpret_cast<char*>(buffer.data()), d[0], d[1], num_channels, &buffer_len);
    if (!png_buffer) {
        return false;
    }

    stream.write(static_cast<char*>(png_buffer), buffer_len);

    mz_free(png_buffer);

    return true;
}

byte4 float_to_rgbe(float4 const& c) {
    float v = c[0];

    if (c[1] > v) {
        v = c[1];
    }

    if (c[2] > v) {
        v = c[2];
    }

    if (v < 1e-32f) {
        return byte4(0, 0, 0, 0);
    }
    int         e;
    float const f = std::frexp(v, &e);

    v = f * 256.f / v;

    return byte4(uint8_t(c[0] * v), uint8_t(c[1] * v), uint8_t(c[2] * v), uint8_t(e + 128));
}

byte4 float_to_rgbd(float4 const& c, float max) {
    float v = c[0];

    if (c[1] > v) {
        v = c[1];
    }

    if (c[2] > v) {
        v = c[2];
    }

    if (v < 1e-32f) {
        return byte4(0, 0, 0, 0);
    }

    float d = std::max(max / v, 1.f);

  //  d = saturate(std::floor(d) / 255.f);

    if (d > 1.f) {
        std::cout << d << std::endl;
    }

    int         e;
    float const f = std::frexp(v, &e);

    v = f * 256.f / v;

    return byte4(uint8_t(c[0] * v), uint8_t(c[1] * v), uint8_t(c[2] * v), uint8_t(e + 128));
}

}  // namespace image::encoding::rgbe
