#include "tonemapper.hpp"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "base/spectrum/rgb.hpp"
#include "base/thread/thread_pool.hpp"
#include "image/typed_image.hpp"

#include <cmath>

namespace rendering::postprocessor::tonemapping {

Tonemapper::Tonemapper(bool auto_expose, float exposure)
    : auto_expose_(auto_expose), exposure_factor_(std::exp2(exposure)) {}

Tonemapper::~Tonemapper() = default;

void Tonemapper::init(Camera const& /*camera*/, thread::Pool& /*threads*/) {}

void Tonemapper::pre_apply(image::Float4 const& source, image::Float4& /*destination*/,
                           thread::Pool&        threads) {
    if (!auto_expose_) {
        return;
    }

    struct Luminance {
        float max;
        float average;
    };

    memory::Buffer<Luminance> luminances(threads.num_threads());

    threads.run_range(
        [&source, &luminances](uint32_t id, int32_t begin, int32_t end) {
            float const iaf = 1.f / float(source.description().area());

            float max     = 0.f;
            float average = 0.f;

            for (int32_t i = begin; i < end; ++i) {
                float3 const color = source.at(i).xyz();

                float const luminance = spectrum::luminance(color);

                max = std::max(max, luminance);

                average += luminance * iaf;
            }

            luminances[id].max     = max;
            luminances[id].average = average;
        },
        0, source.description().area());

    float max_luminance     = 0.f;
    float average_luminance = 0.f;

    for (uint32_t i = 0, len = threads.num_threads(); i < len; ++i) {
        max_luminance = std::max(max_luminance, luminances[i].max);
        average_luminance += luminances[i].average;
    }

    static float constexpr Gray = 0.18f;

    float const factor = Gray / average_luminance;

    exposure_factor_ = factor;
}

float Tonemapper::normalization_factor(float linear_max, float tonemapped_max) {
    return linear_max > 0.f ? 1.f / tonemapped_max : 1.f;
}

}  // namespace rendering::postprocessor::tonemapping
