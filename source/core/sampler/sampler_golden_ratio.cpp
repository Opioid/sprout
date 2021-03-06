#include "sampler_golden_ratio.hpp"
#include "base/math/sample_distribution.inl"
#include "base/math/vector2.inl"
#include "base/random/generator.inl"
#include "base/random/shuffle.hpp"
#include "sampler.inl"

#include "base/debug/assert.hpp"

namespace sampler {

Golden_ratio::Golden_ratio(uint32_t num_dimensions_2D, uint32_t num_dimensions_1D,
                           uint32_t max_samples)
    : Buffered(num_dimensions_2D, num_dimensions_1D, max_samples),
      samples_(new float[max_samples * (2 * num_dimensions_2D + num_dimensions_1D)]) {}

Golden_ratio::~Golden_ratio() {
    delete[] samples_;
}

float2 Golden_ratio::sample_2D(RNG& rng, uint32_t dimension) {
    SOFT_ASSERT(current_sample_[dimension] < num_samples_);

    uint32_t const current = current_sample_[dimension]++;

    if (0 == current) {
        generate_2D(rng, dimension);
    }

    float2 const* samples_2D = reinterpret_cast<float2*>(samples_);

    return samples_2D[dimension * num_samples_ + current];
}

float Golden_ratio::sample_1D(RNG& rng, uint32_t dimension) {
    SOFT_ASSERT(current_sample_[num_dimensions_2D_ + dimension] < num_samples_);

    uint32_t const current = current_sample_[num_dimensions_2D_ + dimension]++;

    if (0 == current) {
        generate_1D(rng, dimension);
    }

    float const* samples_1D = samples_ + num_samples_ * 2 * num_dimensions_2D_;

    return samples_1D[dimension * num_samples_ + current];
}

void Golden_ratio::on_start_pixel(rnd::Generator& /*rng*/) {}

void Golden_ratio::generate_2D(RNG& rng, uint32_t dimension) {
    float2 const r(rng.random_float(), rng.random_float());

    uint32_t const num_samples = num_samples_;

    float2* begin = reinterpret_cast<float2*>(samples_) + dimension * num_samples;

    golden_ratio(begin, num_samples, r);

    rnd::biased_shuffle(begin, num_samples, rng);
}

void Golden_ratio::generate_1D(RNG& rng, uint32_t dimension) {
    float const r = rng.random_float();

    uint32_t const num_samples = num_samples_;

    float* begin = samples_ + num_samples * (2 * num_dimensions_2D_ + dimension);

    golden_ratio(begin, num_samples, r);

    rnd::biased_shuffle(begin, num_samples, rng);
}

template class Typed_pool<Golden_ratio>;

}  // namespace sampler
