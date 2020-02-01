#include "postprocessor_bloom.hpp"
#include "base/math/filter/gaussian.hpp"
#include "base/math/vector4.inl"
#include "base/spectrum/rgb.hpp"
#include "image/typed_image.hpp"
#include "scene/camera/camera.hpp"

namespace rendering::postprocessor {

Bloom::Bloom(float angle, float alpha, float threshold, float intensity)
    : Postprocessor(2),
      angle_(angle),
      alpha_(alpha),
      threshold_(threshold),
      intensity_(intensity),
      scratch_(image::Description()) {}

void Bloom::init(Camera const& camera, thread::Pool& /*threads*/) {
    scratch_.resize(camera.sensor_dimensions());

    float const solid_angle = camera.pixel_solid_angle();

    int32_t const radius = int32_t(angle_ / solid_angle + 0.5f);

    int32_t const width = 2 * radius + 1;

    kernel_.resize(width);

    float const fr = float(radius);

    math::filter::Gaussian_functor gauss(fr * fr, alpha_);

    for (int32_t x = 0; x < width; ++x) {
        int32_t const o = -radius + x;

        float const fo = float(o);
        float const w  = gauss(fo * fo);

        kernel_[x] = K{o, w};
    }
}

void Bloom::apply(uint32_t /*id*/, uint32_t pass, int32_t begin, int32_t end,
                  image::Float4 const& source, image::Float4& destination) {
    float threshold = threshold_;
    float intensity = intensity_;

    auto const d = destination.description().dimensions_3();

    if (0 == pass) {
        for (int32_t i = begin; i < end; ++i) {
            int2 const c = source.coordinates_2(i);

            float3 accum(0.f);
            float  weight_sum = 0.f;
            for (auto& k : kernel_) {
                int32_t const kx = c[0] + k.o;

                if (kx >= 0 && kx < d[0]) {
                    float3 const color = source.load(kx, c[1]).xyz();

                    float const l = spectrum::luminance(color);

                    if (l > threshold) {
                        accum += k.w * color;
                    }

                    weight_sum += k.w;
                }
            }

            if (weight_sum > 0.f) {
                float3 const bloom = accum / weight_sum;
                scratch_.store(i, float4(bloom));
            } else {
                scratch_.store(i, float4(0.f));
            }
        }
    } else {
        for (int32_t i = begin; i < end; ++i) {
            int2 const c = source.coordinates_2(i);

            float3 accum(0.f);
            float  weight_sum = 0.f;
            for (auto& k : kernel_) {
                int32_t const ky = c[1] + k.o;

                if (ky >= 0 && ky < d[1]) {
                    float3 const bloom = scratch_.load(c[0], ky).xyz();
                    accum += k.w * bloom;
                    weight_sum += k.w;
                }
            }

            float4 const s = source.load(i);

            if (weight_sum > 0.f) {
                float3 const bloom = accum / weight_sum;
                destination.store(i, float4(s.xyz() + intensity * bloom, s[3]));
            } else {
                destination.store(i, s);
            }
        }
    }
}

}  // namespace rendering::postprocessor
