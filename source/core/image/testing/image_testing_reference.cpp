#include "image_testing_reference.hpp"
#include <fstream>
#include "base/spectrum/rgb.hpp"
#include "miniz/miniz.hpp"

namespace image {
namespace testing {

// the normal map will look correct if tangent follows same direction as texture coordinates
// positive z is pointing into the screen

// [0, 0]-----[1, 0]
// |			   |
// |			   |
// |			   |
// |			   |
// |			   |
// [0, 1]-----[1, 1]

// tangent		[1,  0,  0]
// bi-tangent	[0, -1,  0]
// normal       [0,  0, -1]

float3 reference_normal(float2 p, float2 range) {
    float vx = -1.f + p[0] * range[0];
    float vy = -1.f + p[1] * range[1];

    float2 xy(vx, vy);
    float  l = math::length(xy);

    float3 v;

    float const radius = 1.f - 0.5f * range[1];
    if (l < radius) {
        v = float3(vx, vy, 1.f - l);
    } else {
        v = float3(0.f, 0.f, 1.f);
    }

    //	return math::normalize(v);

    v = math::normalize(v);

    return 0.5f * float3(v[0] + 1.f, v[1] + 1.f, v[2] + 1.f);
}

void create_reference_normal_map(int2 dimensions) {
    byte3* rgb = new byte3[dimensions[0] * dimensions[1]];

    float2 range(2.f / static_cast<float>(dimensions[0]), 2.f / static_cast<float>(dimensions[1]));

    int2 aa(16, 16);

    float2 aa_delta  = 1.f / float2(aa);
    float2 aa_offset = 0.5f * aa_delta;

    for (int32_t y = 0; y < dimensions[1]; ++y) {
        for (int32_t x = 0; x < dimensions[0]; ++x) {
            auto& pixel = rgb[y * dimensions[0] + x];

            float fx = static_cast<float>(x);
            float fy = static_cast<float>(y);

            float3 v = float3::identity();

            for (int32_t ay = 0; ay < aa[1]; ++ay) {
                for (int32_t ax = 0; ax < aa[0]; ++ax) {
                    float2 p(fx + aa_offset[0] + static_cast<float>(ax) * aa_delta[0],
                             fy + aa_offset[1] + static_cast<float>(ay) * aa_delta[1]);

                    v += reference_normal(p, range);
                }
            }

            //			v = math::normalize(v / static_cast<float>(aa.x * aa.y));

            //			pixel.x = spectrum::float_to_snorm(v.x);
            //			pixel.y = spectrum::float_to_snorm(v.y);
            //			pixel.z = spectrum::float_to_snorm(v.z);

            v = v / static_cast<float>(aa[0] * aa[1]);

            pixel[0] = static_cast<uint8_t>(v[0] * 255.f);
            pixel[1] = static_cast<uint8_t>(v[1] * 255.f);
            pixel[2] = static_cast<uint8_t>(v[2] * 255.f);
        }
    }

    std::ofstream stream("reference_normal.png", std::ios::binary);
    if (!stream) {
        return;
    }

    size_t buffer_len = 0;
    void*  png_buffer =
        tdefl_write_image_to_png_file_in_memory(rgb, dimensions[0], dimensions[1], 3, &buffer_len);

    if (!png_buffer) {
        delete[] rgb;
        return;
    }

    stream.write(static_cast<char*>(png_buffer), buffer_len);

    mz_free(png_buffer);

    delete[] rgb;
}

}  // namespace testing
}  // namespace image
