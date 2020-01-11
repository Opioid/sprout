#include "sampler_golden_ratio.hpp"
#include "base/math/sample_distribution.inl"
#include "base/math/vector2.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "base/random/shuffle.hpp"
#include "camera_sample.hpp"
#include "sampler.inl"

#include "base/debug/assert.hpp"

namespace sampler {

Golden_ratio::Golden_ratio(rnd::Generator& rng) noexcept
    : Sampler(rng), samples_2D_(nullptr), samples_1D_(nullptr) {}

Golden_ratio::~Golden_ratio() noexcept {
    memory::free_aligned(samples_2D_);
}

Camera_sample Golden_ratio::generate_camera_sample(int2 pixel, uint32_t index) noexcept {
    SOFT_ASSERT(index < num_samples_);

    if (0 == index) {
        generate_2D(0);
        generate_2D(1);
        generate_1D(0);
    }

    return Camera_sample{pixel, samples_2D_[index], samples_2D_[num_samples_ + index],
                         samples_1D_[index]};
}

float2 Golden_ratio::generate_sample_2D(uint32_t dimension) noexcept {
    SOFT_ASSERT(current_sample_2D_[dimension] < num_samples_);

    uint32_t const current = current_sample_2D_[dimension]++;

    if (0 == current) {
        generate_2D(dimension);
    }

    return samples_2D_[dimension * num_samples_ + current];
}

float Golden_ratio::generate_sample_1D(uint32_t dimension) noexcept {
    SOFT_ASSERT(current_sample_1D_[dimension] < num_samples_);

    uint32_t const current = current_sample_1D_[dimension]++;

    if (0 == current) {
        generate_1D(dimension);
    }

    return samples_1D_[dimension * num_samples_ + current];
}

void Golden_ratio::on_resize() noexcept {
    memory::free_aligned(samples_2D_);

    float* buffer = memory::allocate_aligned<float>(num_samples_ * 2 * num_dimensions_2D_ +
                                                    num_samples_ * num_dimensions_1D_);

    samples_2D_ = reinterpret_cast<float2*>(buffer);
    samples_1D_ = buffer + num_samples_ * 2 * num_dimensions_2D_;
}

void Golden_ratio::on_start_pixel() noexcept {}

void Golden_ratio::generate_2D(uint32_t dimension) noexcept {
    float2 const r(rng_.random_float(), rng_.random_float());

    float2* begin = samples_2D_ + dimension * num_samples_;

    golden_ratio(begin, num_samples_, r);

    rnd::biased_shuffle(begin, num_samples_, rng_);
}

void Golden_ratio::generate_1D(uint32_t dimension) noexcept {
    float const r = rng_.random_float();

    float* begin = samples_1D_ + dimension * num_samples_;

    golden_ratio(begin, num_samples_, r);

    rnd::biased_shuffle(begin, num_samples_, rng_);
}

template class Typed_pool<Golden_ratio>;

}  // namespace sampler
