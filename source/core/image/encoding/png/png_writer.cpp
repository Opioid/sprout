#include "png_writer.hpp"
#include <fstream>
#include <vector>
#include "base/math/vector4.inl"
#include "base/thread/thread_pool.hpp"
#include "image/typed_image.hpp"
#include "miniz/miniz.hpp"

namespace image::encoding::png {

Writer::Writer(int2 dimensions, bool error_diffusion) : Srgb(dimensions, error_diffusion) {}

std::string Writer::file_extension() const {
    return "png";
}

bool Writer::write(std::ostream& stream, Float4 const& image, thread::Pool& pool) {
    auto const d = image.description().dimensions;

    pool.run_range(
        [this, &image](uint32_t /*id*/, int32_t begin, int32_t end) { to_sRGB(image, begin, end); },
        0, d[1]);

    size_t buffer_len = 0;
    void*  png_buffer = tdefl_write_image_to_png_file_in_memory(rgb_, d[0], d[1], 3, &buffer_len);
    if (!png_buffer) {
        return false;
    }

    stream.write(static_cast<char*>(png_buffer), buffer_len);

    mz_free(png_buffer);

    return true;
}

bool Writer::write(std::string_view name, Byte3 const& image) {
    std::ofstream stream(name.data(), std::ios::binary);
    if (!stream) {
        return false;
    }

    auto const d = image.description().dimensions;

    size_t buffer_len = 0;
    void*  png_buffer = tdefl_write_image_to_png_file_in_memory(image.data(), d[0], d[1], 3,
                                                               &buffer_len);

    if (!png_buffer) {
        return false;
    }

    stream.write(static_cast<char*>(png_buffer), buffer_len);

    mz_free(png_buffer);

    return true;
}

bool Writer::write(std::string_view name, Byte1 const& image) {
    std::ofstream stream(name.data(), std::ios::binary);
    if (!stream) {
        return false;
    }

    auto const d = image.description().dimensions;

    size_t buffer_len = 0;
    void*  png_buffer = tdefl_write_image_to_png_file_in_memory(image.data(), d[0], d[1], 1,
                                                               &buffer_len);

    if (!png_buffer) {
        return false;
    }

    stream.write(static_cast<char*>(png_buffer), buffer_len);

    mz_free(png_buffer);

    return true;
}

bool Writer::write(std::string_view name, float const* data, int2 dimensions, float scale) {
    std::ofstream stream(name.data(), std::ios::binary);
    if (!stream) {
        return false;
    }

    uint32_t const       area = static_cast<uint32_t>(dimensions[0] * dimensions[1]);
    std::vector<uint8_t> bytes(area);

    for (uint32_t i = 0; i < area; ++i) {
        bytes[i] = static_cast<uint8_t>(scale * data[i]);
    }

    size_t buffer_len = 0;
    void*  png_buffer = tdefl_write_image_to_png_file_in_memory(bytes.data(), dimensions[0],
                                                               dimensions[1], 1, &buffer_len);

    if (!png_buffer) {
        return false;
    }

    stream.write(static_cast<char*>(png_buffer), buffer_len);

    mz_free(png_buffer);

    return true;
}

bool Writer::write(std::string_view name, float2 const* data, int2 dimensions, float scale) {
    std::ofstream stream(name.data(), std::ios::binary);
    if (!stream) {
        return false;
    }

    uint32_t const     area = static_cast<uint32_t>(dimensions[0] * dimensions[1]);
    std::vector<byte3> bytes(area);

    for (uint32_t i = 0; i < area; ++i) {
        bytes[i] = byte3(static_cast<uint8_t>(scale * data[i][0]),
                         static_cast<uint8_t>(scale * data[i][1]), 0);
    }

    size_t buffer_len = 0;
    void*  png_buffer = tdefl_write_image_to_png_file_in_memory(bytes.data(), dimensions[0],
                                                               dimensions[1], 3, &buffer_len);

    if (!png_buffer) {
        return false;
    }

    stream.write(static_cast<char*>(png_buffer), buffer_len);

    mz_free(png_buffer);

    return true;
}

bool Writer::write(std::string_view name, packed_float3 const* data, int2 dimensions, float scale) {
    std::ofstream stream(name.data(), std::ios::binary);
    if (!stream) {
        return false;
    }

    uint32_t const     area = static_cast<uint32_t>(dimensions[0] * dimensions[1]);
    std::vector<byte3> bytes(area);

    for (uint32_t i = 0; i < area; ++i) {
        bytes[i] = byte3(static_cast<uint8_t>(scale * data[i][0]),
                         static_cast<uint8_t>(scale * data[i][1]),
                         static_cast<uint8_t>(scale * data[i][2]));
    }

    size_t buffer_len = 0;
    void*  png_buffer = tdefl_write_image_to_png_file_in_memory(bytes.data(), dimensions[0],
                                                               dimensions[1], 3, &buffer_len);

    if (!png_buffer) {
        return false;
    }

    stream.write(static_cast<char*>(png_buffer), buffer_len);

    mz_free(png_buffer);

    return true;
}

Writer_alpha::Writer_alpha(int2 dimensions, bool error_diffusion)
    : Srgb_alpha(dimensions, error_diffusion) {}

std::string Writer_alpha::file_extension() const {
    return "png";
}

bool Writer_alpha::write(std::ostream& stream, Float4 const& image, thread::Pool& pool) {
    auto const d = image.description().dimensions;

    pool.run_range(
        [this, &image](uint32_t /*id*/, int32_t begin, int32_t end) { to_sRGB(image, begin, end); },
        0, d[1]);

    size_t buffer_len = 0;
    void*  png_buffer = tdefl_write_image_to_png_file_in_memory(rgba_, d[0], d[1], 4, &buffer_len);

    if (!png_buffer) {
        return false;
    }

    stream.write(static_cast<char*>(png_buffer), buffer_len);

    mz_free(png_buffer);

    return true;
}

}  // namespace image::encoding::png
