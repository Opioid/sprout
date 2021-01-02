#include "sampler_rd.hpp"
#include "base/math/math.hpp"
#include "base/math/vector2.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "base/random/shuffle.hpp"
#include "sampler.inl"

// http://extremelearning.com.au/unreasonable-effectiveness-of-quasirandom-sequences/

namespace sampler {

static inline float r1(float seed, float n) {
    static float constexpr g = 1.61803398874989484820458683436563f;

    static float constexpr a1 = 1.f / g;

    return frac(seed + a1 * n);
}

static inline float2 r2(float2 seed, float n) {
    static float constexpr g = 1.32471795724474602596090885447809f;

    static float constexpr a1 = 1.f / g;
    static float constexpr a2 = 1.f / (g * g);

    return float2(frac(seed[0] + a1 * n), frac(seed[1] + a2 * n));
}

static inline float2 r2i(float2 seed, uint32_t n) {
    // https://www.shadertoy.com/view/4dtBWH

    static float const e = std::exp2(24.f);

    return float2(frac(seed[0] + float(n * 12664745) / e), frac(seed[1] + float(n * 9560333) / e));
}

RD::RD(uint32_t num_dimensions_2D, uint32_t num_dimensions_1D, uint32_t max_samples)
    : Buffered(num_dimensions_2D, num_dimensions_1D, max_samples) {
    float* seeds = memory::allocate_aligned<float>(2 * num_dimensions_2D + num_dimensions_1D);

    seeds_2D_ = reinterpret_cast<float2*>(seeds);
    seeds_1D_ = seeds + 2 * num_dimensions_2D;

    float* buffer = memory::allocate_aligned<float>(Num_batch * 2 * num_dimensions_2D +
                                                    Num_batch * num_dimensions_1D);

    samples_2D_ = reinterpret_cast<float2*>(buffer);
    samples_1D_ = buffer + Num_batch * 2 * num_dimensions_2D_;

    consumed_2D_ = memory::allocate_aligned<uint32_t>(num_dimensions_2D + num_dimensions_1D);

    consumed_1D_ = consumed_2D_ + num_dimensions_2D;
}

RD::~RD() {
    std::free(consumed_2D_);
    std::free(samples_2D_);
    std::free(seeds_2D_);
}

float2 RD::sample_2D(RNG& rng, uint32_t dimension) {
    if (Num_batch == consumed_2D_[dimension]) {
        generate_2D(rng, dimension);
    }

    uint32_t const current = consumed_2D_[dimension]++;

    return samples_2D_[dimension * Num_batch + current];
}

float RD::sample_1D(RNG& rng, uint32_t dimension) {
    if (Num_batch == consumed_1D_[dimension]) {
        generate_1D(rng, dimension);
    }

    uint32_t const current = consumed_1D_[dimension]++;

    return samples_1D_[dimension * Num_batch + current];
}

void RD::on_start_pixel(RNG& rng) {
    for (uint32_t i = 0, len = num_dimensions_2D_; i < len; ++i) {
        seeds_2D_[i] = float2(rng.random_float(), rng.random_float());
    }

    for (uint32_t i = 0, len = num_dimensions_1D_; i < len; ++i) {
        seeds_1D_[i] = rng.random_float();
    }

    for (uint32_t i = 0, len = num_dimensions_2D_ + num_dimensions_1D_; i < len; ++i) {
        consumed_2D_[i] = Num_batch;
    }
}

void RD::generate_2D(RNG& rng, uint32_t dimension) {
    float2 const seed = seeds_2D_[dimension];

    float2* begin = samples_2D_ + dimension * Num_batch;

    uint32_t& current_sample = current_sample_[dimension];

    for (uint32_t i = 0; i < Num_batch; ++i) {
        uint32_t const n = ++current_sample;

        begin[i] = r2i(seed, n);
    }

    rnd::biased_shuffle(begin, Num_batch, rng);

    consumed_2D_[dimension] = 0;
}

void RD::generate_1D(RNG& rng, uint32_t dimension) {
    float const seed = seeds_1D_[dimension];

    float* begin = samples_1D_ + dimension * Num_batch;

    uint32_t& current_sample = current_sample_[num_dimensions_2D_ + dimension];

    for (uint32_t i = 0; i < Num_batch; ++i) {
        float const n = float(++current_sample);

        begin[i] = r1(seed, n);
    }

    rnd::biased_shuffle(begin, Num_batch, rng);

    consumed_1D_[dimension] = 0;
}

template class Typed_pool<RD>;

}  // namespace sampler
