#include "image_testing_reference.hpp"
#include "base/encoding/encoding.inl"
#include "base/spectrum/rgb.hpp"
#include "miniz/miniz.h"

#include <iostream>

#include <fstream>

namespace image::testing {

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

static float3 reference_normal(float2 p, float range) {
    float vx = -1.f + p[0] * range;
    float vy = -1.f + p[1] * range;

    float2 xy(vx, vy);
    float  l = length(xy);

    float3 v;

    float const radius = 1.f - 0.5f * range;
    if (l < radius) {
        v = float3(vx, vy, 1.f - l);
    } else {
        v = float3(0.f, 0.f, 1.f);
    }

    return normalize(v);

    v = normalize(v);

    return 0.5f * (v + 1.f);
}

void create_reference_normal_map(int32_t dimensions, std::string const& name) {
    byte3* rgb = new byte3[dimensions * dimensions];

    float range = 2.f / float(dimensions);

    int2 aa(16, 16);

    float2 aa_delta  = 1.f / float2(aa);
    float2 aa_offset = 0.5f * aa_delta;

    for (int32_t y = 0; y < dimensions; ++y) {
        for (int32_t x = 0; x < dimensions; ++x) {
            auto& pixel = rgb[y * dimensions + x];

            float fx = float(x);
            float fy = float(y);

            float3 v = float3(0.f);

            for (int32_t ay = 0; ay < aa[1]; ++ay) {
                for (int32_t ax = 0; ax < aa[0]; ++ax) {
                    float2 p(fx + aa_offset[0] + float(ax) * aa_delta[0],
                             fy + aa_offset[1] + float(ay) * aa_delta[1]);

                    v += reference_normal(p, range);
                }
            }

            //			v = normalize(v / float(aa.x * aa.y));

            //			pixel.x = spectrum::float_to_snorm(v.x);
            //			pixel.y = spectrum::float_to_snorm(v.y);
            //			pixel.z = spectrum::float_to_snorm(v.z);

            v = v / float(aa[0] * aa[1]);

            pixel[0] = encoding::float_to_snorm(v[0]);
            pixel[1] = encoding::float_to_snorm(v[1]);
            pixel[2] = encoding::float_to_snorm(v[2]);
        }
    }

    std::ofstream stream(name, std::ios::binary);
    if (!stream) {
        return;
    }

    size_t buffer_len = 0;
    void*  png_buffer = tdefl_write_image_to_png_file_in_memory(rgb, dimensions, dimensions, 3,
                                                               &buffer_len);

    if (!png_buffer) {
        delete[] rgb;
        return;
    }

    stream.write(static_cast<char*>(png_buffer), buffer_len);

    mz_free(png_buffer);

    delete[] rgb;
}

}  // namespace image::testing
