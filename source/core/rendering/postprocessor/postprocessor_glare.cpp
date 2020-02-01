#include "postprocessor_glare.hpp"
#include "base/math/exp.hpp"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "base/memory/array.inl"
#include "base/spectrum/interpolated.hpp"
#include "base/spectrum/rgb.hpp"
#include "base/spectrum/xyz.hpp"
#include "base/thread/thread_pool.hpp"
#include "image/typed_image.hpp"
#include "scene/camera/camera.hpp"

namespace rendering::postprocessor {

Glare::Glare(Adaption adaption, float threshold, float intensity)
    : Postprocessor(2),
      adaption_(adaption),
      threshold_(threshold),
      intensity_(intensity),
      kernel_(nullptr) {}

Glare::~Glare() {
    memory::free_aligned(kernel_);
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

void Glare::init(Camera const& camera, thread::Pool& threads) {
    auto const dim = camera.sensor_dimensions();

    dimensions_ = dim;

    uint32_t const buffer_size = uint32_t(dim[0] * dim[1]);

    high_.reserve(buffer_size);

    float const angle = math::radians_to_degrees(std::sqrt(camera.pixel_solid_angle()));

    kernel_ = memory::allocate_aligned<float3>(buffer_size);

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

    memory::Buffer<F> f(static_cast<uint32_t>(buffer_size));

    struct Init {
        float  a_sum = 0.f;
        float  b_sum = 0.f;
        float  c_sum = 0.f;
        float3 d_sum = float3(0.f);
    };

    memory::Array<Init> inits(threads.num_threads());

    threads.run_range(
        [this, angle, wl_norm, &CIE_X, &CIE_Y, &CIE_Z, &f, &inits](uint32_t id, int32_t begin,
                                                                   int32_t end) {
            Init& init = inits[id];

            for (int32_t y = begin; y < end; ++y) {
                for (int32_t x = 0; x < dimensions_[0]; ++x) {
                    float2 const p(float(x) + 0.5f, float(y) + 0.5f);

                    float const theta = length(p) * angle;

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

                    int32_t const i = y * dimensions_[0] + x;

                    f[i] = F{a, b, c, d};
                }
            }
        },
        0, dimensions_[1]);

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

    float const a_n = scale[0] / (4.f * a_sum);
    float const b_n = scale[1] / (4.f * b_sum);
    float const c_n = scale[2] / (4.f * c_sum);

    if (Adaption::Photopic == adaption_) {
        for (uint32_t i = 0, len = buffer_size; i < len; ++i) {
            kernel_[i] = float3(a_n * f[i].a + b_n * f[i].b + c_n * f[i].c);
        }
    } else {
        float3 const d_n = scale[3] / (4.f * d_sum);

        for (uint32_t i = 0, len = buffer_size; i < len; ++i) {
            kernel_[i] = float3(a_n * f[i].a + b_n * f[i].b + c_n * f[i].c) + d_n * f[i].d;
        }
    }
}

void Glare::pre_apply(image::Float4 const& /*source*/, image::Float4& /*destination*/,
                      thread::Pool& /*threads*/) {
    high_.clear();
}

void Glare::apply(uint32_t /*id*/, uint32_t pass, int32_t begin, int32_t end,
                  image::Float4 const& source, image::Float4& destination) {
    if (0 == pass) {
        float threshold = threshold_;

        for (int32_t i = begin; i < end; ++i) {
            float3 const color = source.at(i).xyz();

            float const l = spectrum::luminance(color);

            if (l > threshold) {
                int2 const c = destination.coordinates_2(i);
                high_.push_back(c);
            }
        }
    } else {
        Simd3f const intensity(intensity_);

        int32_t const d0 = destination.description().dimensions_2()[0];

        for (int32_t i = begin; i < end; ++i) {
            int2 const kb = -destination.coordinates_2(i);

            Simd3f glare(simd::Zero);
            for (int2 hc : high_) {
                int2 const kc = abs(kb + hc);

                int32_t const ki = kc[1] * d0 + kc[0];

                Simd3f const k(kernel_[ki].v);
                Simd3f const h(source.at(hc[0], hc[1]).v);

                glare += k * h;
            }

            glare *= intensity;

            Simd3f s(reinterpret_cast<float*>(source.address(i)));

            s += glare;

            simd::store_float4(reinterpret_cast<float*>(destination.address(i)), s.v);
        }
    }
}

void Glare::post_pass(uint32_t pass) {
    if (0 == pass) {
        std::sort(high_.begin(), high_.end());
    }
}

}  // namespace rendering::postprocessor
