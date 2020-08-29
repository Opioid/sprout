#include "sampler_golden_ratio.hpp"
#include "base/math/sample_distribution.inl"
#include "base/math/vector2.inl"
#include "base/random/generator.inl"
#include "base/random/shuffle.hpp"
#include "sampler.inl"

#include "base/debug/assert.hpp"

namespace sampler {

Golden_ratio::Golden_ratio() : samples_(nullptr) {}

Golden_ratio::~Golden_ratio() {
    delete[] samples_;
}

float2 Golden_ratio::generate_sample_2D(rnd::Generator& rng, uint32_t dimension) {
    SOFT_ASSERT(current_sample_[dimension] < num_samples_);

    uint32_t const current = current_sample_[dimension]++;

    if (0 == current) {
        generate_2D(rng, dimension);
    }

    float2 const* samples_2D = reinterpret_cast<float2*>(samples_);

    return samples_2D[dimension * num_samples_ + current];
}

float Golden_ratio::generate_sample_1D(rnd::Generator& rng, uint32_t dimension) {
    SOFT_ASSERT(current_sample_[num_dimensions_2D_ + dimension] < num_samples_);

    uint32_t const current = current_sample_[num_dimensions_2D_ + dimension]++;

    if (0 == current) {
        generate_1D(rng, dimension);
    }

    float const* samples_1D = samples_ + num_samples_ * 2 * num_dimensions_2D_;

    return samples_1D[dimension * num_samples_ + current];
}

void Golden_ratio::on_resize() {
    delete[] samples_;

    samples_ = new float[num_samples_ * 2 * num_dimensions_2D_ + num_samples_ * num_dimensions_1D_];
}

void Golden_ratio::on_start_pixel(rnd::Generator& /*rng*/) {}

void Golden_ratio::generate_2D(rnd::Generator& rng, uint32_t dimension) {
    float2 const r(rng.random_float(), rng.random_float());

    float2* begin = reinterpret_cast<float2*>(samples_) + dimension * num_samples_;

    golden_ratio(begin, num_samples_, r);

    rnd::biased_shuffle(begin, num_samples_, rng);
}

void Golden_ratio::generate_1D(rnd::Generator& rng, uint32_t dimension) {
    float const r = rng.random_float();

    float* begin = samples_ + num_samples_ * 2 * num_dimensions_2D_ + dimension * num_samples_;

    golden_ratio(begin, num_samples_, r);

    rnd::biased_shuffle(begin, num_samples_, rng);
}

template class Typed_pool<Golden_ratio>;

}  // namespace sampler
