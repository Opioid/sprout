#include "postprocessor_glare_stochastic.hpp"
#include "base/math/exp.hpp"
#include "base/math/filter/gaussian.hpp"
#include "base/math/sample_distribution.inl"
#include "base/math/sampling.inl"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "base/memory/array.inl"
#include "base/random/generator.inl"
#include "base/spectrum/interpolated.hpp"
#include "base/spectrum/rgb.hpp"
#include "base/spectrum/xyz.hpp"
#include "base/thread/thread_pool.hpp"
#include "image/typed_image.hpp"

namespace rendering::postprocessor {

Glare_stochastic::Glare_stochastic(Adaption adaption, float threshold, float intensity)
    : Postprocessor(2),
      adaption_(adaption),
      threshold_(threshold),
      intensity_(intensity),
      high_pass_(nullptr),
      kernel_(nullptr),
      gauss_kernel_(nullptr) {}

Glare_stochastic::~Glare_stochastic() {
    memory::free_aligned(gauss_kernel_);
    memory::free_aligned(kernel_);
    memory::free_aligned(high_pass_);
}

static inline float f0(float theta) {
    float b = theta / 0.02f;
    return 2.61f * 10e6f * math::exp(-(b * b));
}

static inline float f1(float theta) {
    float b = 1.f / (theta + 0.02f);
    return 20.91f * b * b * b;
}

static inline float f2(float theta) {
    float b = 1.f / (theta + 0.02f);
    return 72.37f * b * b;
}

static inline float f3(float theta, float lambda) {
    float b = theta - 3.f * (lambda / 568.f);
    return 436.9f * (568.f / lambda) * math::exp(-(b * b));
}

void Glare_stochastic::init(scene::camera::Camera const& camera, thread::Pool& pool) {
    auto const dim = camera.sensor_dimensions();

    dimensions_ = dim;
    high_pass_  = memory::allocate_aligned<float3>(uint32_t(dim[0] * dim[1]));

    // This seems a bit arbitrary
    float const solid_angle = math::radians_to_degrees(std::sqrt(camera.pixel_solid_angle()));

    kernel_dimensions_ = 2 * dim;

    uint32_t const kernel_size = uint32_t(kernel_dimensions_[0] * kernel_dimensions_[1]);

    kernel_ = memory::allocate_aligned<float3>(kernel_size);

    spectrum::Interpolated const CIE_X(spectrum::CIE_XYZ_Num, spectrum::CIE_Wavelengths_360_830_1nm,
                                       spectrum::CIE_X_360_830_1nm);
    spectrum::Interpolated const CIE_Y(spectrum::CIE_XYZ_Num, spectrum::CIE_Wavelengths_360_830_1nm,
                                       spectrum::CIE_Y_360_830_1nm);
    spectrum::Interpolated const CIE_Z(spectrum::CIE_XYZ_Num, spectrum::CIE_Wavelengths_360_830_1nm,
                                       spectrum::CIE_Z_360_830_1nm);

    static float constexpr wl_start = 400.f;
    static float constexpr wl_end   = 700.f;

    static int32_t constexpr wl_num_samples = 64;

    static float constexpr wl_step = (wl_end - wl_start) / float(wl_num_samples);

    float const wl_norm = 1.f / CIE_Y.integrate(wl_start, wl_end);

    struct F {
        float  a;
        float  b;
        float  c;
        float3 d;
    };

    memory::Buffer<F> f(kernel_size);

    struct Init {
        float  a_sum = 0.f;
        float  b_sum = 0.f;
        float  c_sum = 0.f;
        float3 d_sum = float3(0.f);
    };

    memory::Array<Init> inits(pool.num_threads());

    pool.run_range(
        [this, dim, solid_angle, wl_norm, &CIE_X, &CIE_Y, &CIE_Z, &f, &inits](
            uint32_t id, int32_t begin, int32_t end) {
            Init& init = inits[id];

            for (int32_t y = begin; y < end; ++y) {
                for (int32_t x = 0; x < kernel_dimensions_[0]; ++x) {
                    int2 const p(-dim[0] + x, -dim[1] + y);

                    float const theta = length(float2(p)) * solid_angle;

                    float const a = f0(theta);
                    float const b = f1(theta);
                    float const c = f2(theta);

                    init.a_sum += a;
                    init.b_sum += b;
                    init.c_sum += c;

                    float3 d(0.f);

                    if (Adaption::Photopic != adaption_) {
                        float3 xyz(0.f);
                        for (int32_t k = 0; k < wl_num_samples; ++k) {
                            float const lambda = wl_start + float(k) * wl_step;
                            float const val    = wl_norm * f3(theta, lambda);
                            xyz[0] += CIE_X.evaluate(lambda) * val;
                            xyz[1] += CIE_Y.evaluate(lambda) * val;
                            xyz[2] += CIE_Z.evaluate(lambda) * val;
                        }

                        d = math::max(spectrum::XYZ_to_linear_RGB(xyz), float3(0.f));

                        init.d_sum += d;
                    }

                    int32_t const i = y * kernel_dimensions_[0] + x;

                    f[i] = F{a, b, c, d};
                }
            }
        },
        0, kernel_dimensions_[1]);

    float  a_sum = 0.f;
    float  b_sum = 0.f;
    float  c_sum = 0.f;
    float3 d_sum(0.f);

    for (auto i : inits) {
        a_sum += i.a_sum;
        b_sum += i.b_sum;
        c_sum += i.c_sum;
        d_sum += i.d_sum;
    }

    float scale[4];

    switch (adaption_) {
        case Adaption::Scotopic:
            scale[0] = 0.282f;
            scale[1] = 0.478f;
            scale[2] = 0.207f;
            scale[3] = 0.033f;
            break;
        default:
        case Adaption::Mesopic:
            scale[0] = 0.368f;
            scale[1] = 0.478f;
            scale[2] = 0.138f;
            scale[3] = 0.016f;
            break;
        case Adaption::Photopic:
            scale[0] = 0.383f;
            scale[1] = 0.478f;
            scale[2] = 0.138f;
            scale[3] = 0.f;
            break;
    }

    float a_n = scale[0] / a_sum;
    float b_n = scale[1] / b_sum;
    float c_n = scale[2] / c_sum;

    if (Adaption::Photopic == adaption_) {
        for (uint32_t i = 0, len = kernel_size; i < len; ++i) {
            kernel_[i] = float3(a_n * f[i].a + b_n * f[i].b + c_n * f[i].c);
        }
    } else {
        float3 d_n = scale[3] / d_sum;

        for (uint32_t i = 0, len = kernel_size; i < len; ++i) {
            kernel_[i] = float3(a_n * f[i].a + b_n * f[i].b + c_n * f[i].c) + d_n * f[i].d;
        }
    }

    // Gaussian blur
    float const radius = 4.f;  // float(std::max(dim[0], dim[1])) * 0.00125f;

    gauss_width_ = 2 * std::max(int32_t(radius + 0.5f), 1) + 1;

    gauss_kernel_ = memory::allocate_aligned<K>(uint32_t(gauss_width_));

    float const fr = radius + 0.5f;

    const filter::Gaussian_functor gauss(float(fr * fr), 0.15f);

    int32_t const ir = static_cast<int32_t>(radius);

    for (int32_t x = 0, len = gauss_width_; x < len; ++x) {
        int32_t const o = -ir + x;

        float const fo = float(o);
        float const w  = gauss(fo * fo);

        gauss_kernel_[x] = K{o, w};
    }
}

size_t Glare_stochastic::num_bytes() const {
    return sizeof(*this) + uint32_t(dimensions_[0] * dimensions_[1]) * sizeof(float3) +
           uint32_t(kernel_dimensions_[0] * kernel_dimensions_[1]) * sizeof(float3);
}

void Glare_stochastic::apply(uint32_t id, uint32_t pass, int32_t begin, int32_t end,
                             image::Float4 const& source, image::Float4& destination) {
    if (0 == pass) {
        float const threshold = threshold_;

        for (int32_t i = begin; i < end; ++i) {
            float3 const color = source.at(i).xyz();
            float const  l     = spectrum::luminance(color);

            if (l > threshold) {
                high_pass_[i] = color;
            } else {
                high_pass_[i] = float3(0.f);
            }
        }
    } else if (1 == pass) {
        int2 const d = destination.description().dimensions.xy();

        float2 const fd = float2(d);

        float const ratio = fd[1] / fd[0];

        float const fdm0 = float(d[0] - 1);
        float const fdm1 = float(d[1] - 1);

        uint32_t const num_samples = 4 * 4096;

        float const weight = (fd[0] * fd[1]) / float(num_samples);

        float const intensity = weight * intensity_;

        int32_t const kd0 = kernel_dimensions_[0];

        rnd::Generator rng(0, id);

        for (int32_t i = begin; i < end; ++i) {
            int2 const c  = destination.coordinates_2(i);
            int2 const kb = d - c;

            uint32_t const r = rng.random_uint();

            float3 glare(0.f);

            float weight_sum = 0.f;

            for (uint32_t j = 0; j < num_samples; ++j) {
                float2 const uv = hammersley(j, num_samples, r);

                float2 disk = sample_disk_concentric(uv);

                float w = squared_length(disk);

                disk *= w;//squared_length(disk);

                int32_t  sx = c[0] + int32_t(disk[0] * fdm0);
                int32_t  sy = c[1] + int32_t(disk[1] * fdm0);

//                if (sx >= d[0]) {
//                    sx = c[0] + (d[0] - sx);
//                }

//                if (sy < 0) {
//                    sy = c[1] + sy;
//                }

                if (sx < 0 || sx >= d[0] || sy < 0 || sy >= d[1]) {
                    continue;
                }

//                int32_t const sx = static_cast<int32_t>(uv[0] * fdm0);
//                int32_t const sy = static_cast<int32_t>(uv[1] * fdm1);

                int32_t const si = sy * d[0] + sx;

                int2 const kc = kb + int2(sx, sy);

                int32_t const ki = kc[1] * kd0 + kc[0];

                float3 const k = kernel_[ki];

            //    float const w = 1.f;

                glare +=/* w * */(k * high_pass_[si]);

                weight_sum += w;
            }

            destination.store(i, float4((intensity * glare)));
        }
    } else if (2 == pass) {
        // vertical

        auto const d = destination.description().dimensions.xy();

        for (int32_t i = begin; i < end; ++i) {
            int2 const c = destination.coordinates_2(i);

            float3 accum(0.f);
            float  weight_sum = 0.f;
            for (int32_t j = 0, len = gauss_width_; j < len; ++j) {
                auto const    k  = gauss_kernel_[j];
                int32_t const kx = c[0] + k.o;
                if (kx >= 0 && kx < d[0]) {
                    int32_t const si = c[1] * d[0] + kx;
                    float3 const  v  = destination.at(si).xyz();

                    accum += k.w * v;
                    weight_sum += k.w;
                }
            }

            int32_t const di = c[1] * d[0] + c[0];

            high_pass_[di] = accum / weight_sum;
        }
    } else if (3 == pass) {
        // horizontal

        auto const d = destination.description().dimensions.xy();

        for (int32_t i = begin; i < end; ++i) {
            int2 const c = destination.coordinates_2(i);

            float3 accum(0.f);
            float  weight_sum = 0.f;
            for (int32_t j = 0, len = gauss_width_; j < len; ++j) {
                auto const    k  = gauss_kernel_[j];
                int32_t const ky = c[1] + k.o;
                if (ky >= 0 && ky < d[1]) {
                    int32_t const si = ky * d[0] + c[0];
                    float3 const  v  = high_pass_[si];

                    accum += k.w * v;
                    weight_sum += k.w;
                }
            }

            float4 s = source.load(i);

            destination.store(i, float4(s.xyz() + accum / weight_sum, s[3]));
        }
    }
}

}  // namespace rendering::postprocessor
