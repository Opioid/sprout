#include "png_writer.hpp"
#include "base/encoding/encoding.inl"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "base/spectrum/heatmap.hpp"
#include "base/spectrum/rgb.hpp"
#include "base/thread/thread_pool.hpp"
#include "image/typed_image.hpp"
#include "miniz/miniz.hpp"

#include <fstream>

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

bool Writer::write_heatmap(std::string_view name, float const* data, int2 dimensions,
                           float max_value, thread::Pool& pool) noexcept {
    std::ofstream stream(name.data(), std::ios::binary);
    if (!stream) {
        return false;
    }

    float const im = max_value > 0.f ? 1.f / max_value : 1.f;

    pool.run_range(
        [this, data, im](uint32_t /*id*/, int32_t begin, int32_t end) {
            for (int32_t i = begin; i < end; ++i) {
                float const n = data[i] * im;

                float3 const hm = spectrum::heatmap(n);

                rgb_[i] = ::encoding::float_to_unorm(spectrum::linear_to_gamma_sRGB(hm));
            }
        },
        0, dimensions[0] * dimensions[1]);

    size_t buffer_len = 0;
    void*  png_buffer = tdefl_write_image_to_png_file_in_memory(rgb_, dimensions[0], dimensions[1],
                                                               3, &buffer_len);

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

bool Writer::write(std::string_view name, float const* data, int2 dimensions, float scale,
                   bool srgb) {
    std::ofstream stream(name.data(), std::ios::binary);
    if (!stream) {
        return false;
    }

    uint32_t const area  = uint32_t(dimensions[0] * dimensions[1]);
    uint8_t*       bytes = memory::allocate_aligned<uint8_t>(area);

    for (uint32_t i = 0; i < area; ++i) {
        float scaled = scale * data[i];

        if (srgb) {
            scaled = spectrum::linear_to_gamma_sRGB(scaled);
        }

        bytes[i] = ::encoding::float_to_unorm(scaled);
    }

    size_t buffer_len = 0;
    void*  png_buffer = tdefl_write_image_to_png_file_in_memory(bytes, dimensions[0], dimensions[1],
                                                               1, &buffer_len);

    memory::free_aligned(bytes);

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

    uint32_t const area  = uint32_t(dimensions[0] * dimensions[1]);
    byte3*         bytes = memory::allocate_aligned<byte3>(area);

    for (uint32_t i = 0; i < area; ++i) {
        bytes[i] = byte3(uint8_t(scale * data[i][0]), uint8_t(scale * data[i][1]), 0);
    }

    size_t buffer_len = 0;
    void*  png_buffer = tdefl_write_image_to_png_file_in_memory(bytes, dimensions[0], dimensions[1],
                                                               3, &buffer_len);

    memory::free_aligned(bytes);

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

    uint32_t const area = uint32_t(dimensions[0] * dimensions[1]);

    byte3* bytes = memory::allocate_aligned<byte3>(area);

    for (uint32_t i = 0; i < area; ++i) {
        bytes[i] = byte3(uint8_t(scale * data[i][0]), uint8_t(scale * data[i][1]),
                         uint8_t(scale * data[i][2]));
    }

    size_t buffer_len = 0;
    void*  png_buffer = tdefl_write_image_to_png_file_in_memory(bytes, dimensions[0], dimensions[1],
                                                               3, &buffer_len);

    memory::free_aligned(bytes);

    if (!png_buffer) {
        return false;
    }

    stream.write(static_cast<char*>(png_buffer), buffer_len);

    mz_free(png_buffer);

    return true;
}

bool Writer::write_heatmap(std::string_view name, uint32_t const* data, int2 dimensions) {
    std::ofstream stream(name.data(), std::ios::binary);
    if (!stream) {
        return false;
    }

    uint32_t const area = uint32_t(dimensions[0] * dimensions[1]);

    byte3* bytes = memory::allocate_aligned<byte3>(area);

    uint32_t max_value = 0;
    for (uint32_t i = 0; i < area; ++i) {
        max_value = std::max(data[i], max_value);
    }

    float const im = max_value > 0 ? 1.f / float(max_value) : 1.f;
    for (uint32_t i = 0; i < area; ++i) {
        float const n = float(data[i]) * im;

        float3 const hm = spectrum::heatmap(n);

        bytes[i] = ::encoding::float_to_unorm(spectrum::linear_to_gamma_sRGB(hm));
    }

    size_t buffer_len = 0;
    void*  png_buffer = tdefl_write_image_to_png_file_in_memory(bytes, dimensions[0], dimensions[1],
                                                               3, &buffer_len);

    memory::free_aligned(bytes);

    if (!png_buffer) {
        return false;
    }

    stream.write(static_cast<char*>(png_buffer), buffer_len);

    mz_free(png_buffer);

    return true;
}

bool Writer::write_heatmap(std::string_view name, float const* data, int2 dimensions) {
    uint32_t const area = uint32_t(dimensions[0] * dimensions[1]);

    float max_value = 0.f;
    for (uint32_t i = 0; i < area; ++i) {
        max_value = std::max(data[i], max_value);
    }

    return write_heatmap(name, data, dimensions, max_value);
}

bool Writer::write_heatmap(std::string_view name, float const* data, int2 dimensions,
                           float max_value) {
    std::ofstream stream(name.data(), std::ios::binary);
    if (!stream) {
        return false;
    }

    uint32_t const area = uint32_t(dimensions[0] * dimensions[1]);

    byte3* bytes = memory::allocate_aligned<byte3>(area);

    float const im = max_value > 0.f ? 1.f / max_value : 1.f;
    for (uint32_t i = 0; i < area; ++i) {
        float const n = data[i] * im;

        float3 const hm = spectrum::heatmap(n);

        bytes[i] = ::encoding::float_to_unorm(spectrum::linear_to_gamma_sRGB(hm));
    }

    size_t buffer_len = 0;
    void*  png_buffer = tdefl_write_image_to_png_file_in_memory(bytes, dimensions[0], dimensions[1],
                                                               3, &buffer_len);

    memory::free_aligned(bytes);

    if (!png_buffer) {
        return false;
    }

    stream.write(static_cast<char*>(png_buffer), buffer_len);

    mz_free(png_buffer);

    return true;
}

Writer_alpha::Writer_alpha(int2 dimensions, bool error_diffusion, bool pre_mulitplied_alpha)
    : Srgb_alpha(dimensions, error_diffusion, pre_mulitplied_alpha) {}

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
