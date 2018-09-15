#include "image_renderer.hpp"
#include <vector>
#include "base/encoding/encoding.inl"
#include "base/math/sincos.hpp"
#include "base/math/vector4.inl"
#include "base/spectrum/rgb.hpp"
#include "image/texture/sampler/address_mode.hpp"
#include "image/typed_image.inl"

namespace image::procedural {

Renderer::Renderer(int2 dimensions, int32_t sqrt_num_samples)
    : sqrt_num_samples_(sqrt_num_samples),
      dimensions_(sqrt_num_samples * dimensions),
      dimensions_f_(sqrt_num_samples * dimensions),
      samples_(new float4[sqrt_num_samples * dimensions[0] * sqrt_num_samples * dimensions[1]]),
      brush_(float4(0.f, 0.f, 0.f, 1.f)) {}

Renderer::~Renderer() {
    delete[] samples_;
}

void Renderer::set_brush(float3 const& color) {
    brush_ = float4(color, 1.f);
}

void Renderer::set_brush(float4 const& color) {
    brush_ = color;
}

void Renderer::clear() {
    for (int32_t i = 0, len = dimensions_[0] * dimensions_[1]; i < len; ++i) {
        samples_[i] = brush_;
    }
}

void Renderer::draw_circle(float2 pos, float radius) {
    int2 const sample(pos * dimensions_f_);

    int32_t x   = static_cast<int32_t>(radius * dimensions_f_[0]);
    int32_t y   = 0;
    int32_t err = 0;

    while (x >= y) {
        set_row(sample[0] - x, sample[0] + x, sample[1] + y, brush_);
        set_row(sample[0] - y, sample[0] + y, sample[1] + x, brush_);
        set_row(sample[0] - x, sample[0] + x, sample[1] - y, brush_);
        set_row(sample[0] - y, sample[0] + y, sample[1] - x, brush_);

        y += 1;
        err += 1 + 2 * y;

        if (2 * (err - x) + 1 > 0) {
            x -= 1;
            err += 1 - 2 * x;
        }
    }
}

void Renderer::draw_circle(float2 pos, float radius, int32_t border) {
    int2 const sample(pos * dimensions_f_);

    int32_t x   = static_cast<int32_t>(radius * dimensions_f_[0] + 0.5f) + border;
    int32_t y   = 0;
    int32_t err = 0;

    while (x >= y) {
        set_row(sample[0] - x, sample[0] + x, sample[1] + y, brush_);
        set_row(sample[0] - y, sample[0] + y, sample[1] + x, brush_);
        set_row(sample[0] - x, sample[0] + x, sample[1] - y, brush_);
        set_row(sample[0] - y, sample[0] + y, sample[1] - x, brush_);

        y += 1;
        err += 1 + 2 * y;

        if (2 * (err - x) + 1 > 0) {
            x -= 1;
            err += 1 - 2 * x;
        }
    }
}

void Renderer::draw_bounding_square(float2 pos, float radius) {
    int2 const start = int2((pos - radius) * dimensions_f_ - 0.5f) - int2(1);
    int2 const end   = int2((pos + radius) * dimensions_f_ + 0.5f) + int2(1);

    for (int32_t y = start[1]; y < end[1]; ++y) {
        set_row(start[0], end[0], y, brush_);
    }
}

void Renderer::draw_disk(float2 pos, float3 const& normal, float radius) {
    int2 const start((pos - radius) * dimensions_f_);
    int2 const end((pos + radius) * dimensions_f_);

    for (int32_t y = start[1]; y < end[1]; ++y) {
        for (int32_t x = start[0]; x < end[0]; ++x) {
            float2 const sample((static_cast<float>(x) + 0.5f) / dimensions_f_[0],
                                (static_cast<float>(y) + 0.5f) / dimensions_f_[1]);
            if (intersect_disk(pos, normal, radius, sample)) {
                set_sample(x, y, brush_);
            }
        }
    }
}

void Renderer::draw_disk(float2 pos, float3 const& normal, float radius, int32_t border) {
    int2 const start((pos - radius) * dimensions_f_ - 1.f);
    int2 const end((pos + radius) * dimensions_f_ + 1.5f);

    float const padded_radius = radius + static_cast<float>(border) / dimensions_f_[0];

    for (int32_t y = start[1]; y < end[1]; ++y) {
        for (int32_t x = start[0]; x < end[0]; ++x) {
            float2 const sample((static_cast<float>(x) + 0.5f) / dimensions_f_[0],
                                (static_cast<float>(y) + 0.5f) / dimensions_f_[1]);
            if (intersect_disk(pos, normal, padded_radius, sample)) {
                set_sample(x, y, brush_);
            }
        }
    }
}

void Renderer::draw_n_gon(float2 pos, float angle, float radius, uint32_t num_vertices) {
    int2 const start((pos - radius) * dimensions_f_);
    int2 const end((pos + radius) * dimensions_f_);

    for (int32_t y = start[1]; y < end[1]; ++y) {
        for (int32_t x = start[0]; x < end[0]; ++x) {
            float2 const sample((static_cast<float>(x) + 0.5f) / dimensions_f_[0],
                                (static_cast<float>(y) + 0.5f) / dimensions_f_[1]);
            if (intersect_n_gon(pos, angle, radius, num_vertices, sample)) {
                set_sample(x, y, brush_);
            }
        }
    }
}

void Renderer::resolve_sRGB(Byte3& image) const {
    if (1 == sqrt_num_samples_) {
        for (int32_t i = 0, len = image.area(); i < len; ++i) {
            auto const s = samples_[i];

            byte3 const srgb = encoding::float_to_unorm(spectrum::linear_RGB_to_sRGB(s.xyz()));
            image.store(i, srgb);
        }
    } else {
        int32_t const num_samples = sqrt_num_samples_ * sqrt_num_samples_;

        float const n = 1.f / static_cast<float>(num_samples);

        auto const i_d = image.description().dimensions;

        for (int32_t i_y = 0; i_y < i_d[1]; ++i_y) {
            int32_t const b_y = sqrt_num_samples_ * i_y;
            for (int32_t i_x = 0; i_x < i_d[0]; ++i_x) {
                int32_t const b_x = sqrt_num_samples_ * i_x;

                float3 result(0.f);

                for (int32_t y = 0; y < sqrt_num_samples_; ++y) {
                    int32_t const b_o = dimensions_[0] * (b_y + y) + b_x;
                    for (int32_t x = 0; x < sqrt_num_samples_; ++x) {
                        int32_t const s = b_o + x;
                        result += samples_[s].xyz();
                    }
                }

                byte3 const srgb = encoding::float_to_unorm(
                    spectrum::linear_RGB_to_sRGB(n * result));
                image.store(i_x, i_y, srgb);
            }
        }
    }
}

void Renderer::resolve(Byte3& image) const {
    if (1 == sqrt_num_samples_) {
        for (int32_t i = 0, len = image.area(); i < len; ++i) {
            auto const s = samples_[i];

            image.at(i) = encoding::float_to_snorm(s.xyz());
        }
    } else {
        int32_t const num_samples = sqrt_num_samples_ * sqrt_num_samples_;

        float const n = 1.f / static_cast<float>(num_samples);

        auto const i_d = image.description().dimensions;

        for (int32_t i_y = 0; i_y < i_d[1]; ++i_y) {
            int32_t const b_y = sqrt_num_samples_ * i_y;
            for (int32_t i_x = 0; i_x < i_d[0]; ++i_x) {
                int32_t const b_x = sqrt_num_samples_ * i_x;

                float3 result(0.f);

                for (int32_t y = 0; y < sqrt_num_samples_; ++y) {
                    int32_t b_o = dimensions_[0] * (b_y + y) + b_x;
                    for (int32_t x = 0; x < sqrt_num_samples_; ++x) {
                        int32_t const s = b_o + x;
                        result += samples_[s].xyz();
                    }
                }

                image.at(i_x, i_y) = encoding::float_to_snorm(n * result);
            }
        }
    }
}

void Renderer::resolve(Byte1& image) const {
    if (1 == sqrt_num_samples_) {
        for (int32_t i = 0, len = image.area(); i < len; ++i) {
            auto const s = samples_[i];

            image.at(i) = encoding::float_to_unorm(s[0]);
        }
    } else {
        int32_t const num_samples = sqrt_num_samples_ * sqrt_num_samples_;

        float const n = 1.f / static_cast<float>(num_samples);

        auto const i_d = image.description().dimensions;

        for (int32_t i_y = 0; i_y < i_d[1]; ++i_y) {
            int32_t b_y = sqrt_num_samples_ * i_y;
            for (int32_t i_x = 0; i_x < i_d[0]; ++i_x) {
                int32_t b_x = sqrt_num_samples_ * i_x;

                float result = 0.f;

                for (int32_t y = 0; y < sqrt_num_samples_; ++y) {
                    for (int32_t x = 0; x < sqrt_num_samples_; ++x) {
                        int32_t s = dimensions_[0] * (b_y + y) + b_x + x;

                        result += samples_[s][0];
                    }
                }

                image.at(i_x, i_y) = encoding::float_to_unorm(n * result);
            }
        }
    }
}

void Renderer::set_sample(int32_t x, int32_t y, float4 const& color) {
    x = math::mod(x, dimensions_[0]);
    y = math::mod(y, dimensions_[1]);

    samples_[dimensions_[0] * y + x] = color;
}

void Renderer::set_row(int32_t start_x, int32_t end_x, int32_t y, float4 const& color) {
    for (int32_t x = start_x; x < end_x; ++x) {
        set_sample(x, y, color);
    }
}

bool Renderer::intersect_disk(float2 pos, float3 const& normal, float radius,
                              float2 sample) noexcept {
    float2 const n2 = normal.xy();

    float const d     = math::dot(n2, pos);
    float const denom = normal[2];
    float const numer = math::dot(n2, sample) - d;
    float const hit_t = numer / denom;

    float3 const k = float3(sample - pos, hit_t);

    float const l = math::dot(k, k);

    if (l <= radius * radius) {
        return true;
    }

    return false;
}

bool Renderer::intersect_n_gon(float2 pos, float angle, float radius, uint32_t num_vertices,
                               float2 sample) noexcept {
    float2 const p = sample - pos;

    for (uint32_t i = 0; i < num_vertices; ++i) {
        float const delta = (2.f * math::Pi) / static_cast<float>(num_vertices);
        float const start = static_cast<float>(i) * delta + angle;

        auto const [sin_start, cos_start] = math::sincos(start);

        float2 const b(sin_start, cos_start);

        auto const [sin_start_delta, cos_start_delta] = math::sincos(start + delta);

        float2 c(sin_start_delta, cos_start_delta);

        float2 cb = c - b;

        float2 const n = math::normalize(float2(cb[1], -cb[0]));

        float const id = 1.f / math::dot(n, b);

        float2 const v = id * n;

        float const t = (v[0] * p[0] + v[1] * p[1]);

        if (t > radius) {
            return false;
        }
    }

    return true;
}

}  // namespace image::procedural
