#include "sampler_rd.hpp"
#include "base/math/math.hpp"
#include "base/math/vector2.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "camera_sample.hpp"

// http://extremelearning.com.au/unreasonable-effectiveness-of-quasirandom-sequences/

namespace sampler {

static inline float r1(float seed, float n) noexcept {
    static float constexpr g = 1.61803398874989484820458683436563f;

    static float constexpr a1 = 1.f / g;

    return math::frac(seed + a1 * n);
}

static inline float2 r2(float2 seed, float n) noexcept {
    static float constexpr g = 1.32471795724474602596090885447809f;

    static float constexpr a1 = 1.f / g;
    static float constexpr a2 = 1.f / (g * g);

    return float2(math::frac(seed[0] + a1 * n), math::frac(seed[1] + a2 * n));
}

static inline float2 r2i(float2 seed, uint32_t n) noexcept {
    // https://www.shadertoy.com/view/4dtBWH

    static float const e = std::exp2(24.f);

    return float2(math::frac(seed[0] + float(n * 12664745) / e),
                  math::frac(seed[1] + float(n * 9560333) / e));
}

RD::RD(rnd::Generator& rng) noexcept : Sampler(rng), seeds_2D_(nullptr), seeds_1D_(nullptr) {}

RD::~RD() noexcept {
    memory::free_aligned(seeds_2D_);
}

Camera_sample RD::generate_camera_sample(int2 pixel, uint32_t index) noexcept {
    float2 const seed0 = seeds_2D_[0];
    float2 const seed1 = seeds_2D_[1];
    float const  seed2 = seeds_1D_[0];

    float const n = float(index + 1);

    return Camera_sample{pixel, r2(seed0, n), r2(seed1, n), r1(seed2, n)};
}

float2 RD::generate_sample_2D(uint32_t dimension) noexcept {
    float2 const seed = seeds_2D_[dimension];

    float const n = float(++current_sample_2D_[dimension]);

    return r2(seed, n);
}

float RD::generate_sample_1D(uint32_t dimension) noexcept {
    float const seed = seeds_1D_[dimension];

    float const n = float(++current_sample_1D_[dimension]);

    return r1(seed, n);
}

void RD::on_resize() noexcept {
    memory::free_aligned(seeds_2D_);

    float* seeds = memory::allocate_aligned<float>(2 * num_dimensions_2D_ + num_dimensions_1D_);

    seeds_2D_ = reinterpret_cast<float2*>(seeds);
    seeds_1D_ = seeds + 2 * num_dimensions_2D_;
}

void RD::on_start_pixel() noexcept {
    for (uint32_t i = 0, len = num_dimensions_2D_; i < len; ++i) {
        seeds_2D_[i] = float2(rng_.random_float(), rng_.random_float());
    }

    for (uint32_t i = 0, len = num_dimensions_1D_; i < len; ++i) {
        seeds_1D_[i] = rng_.random_float();
    }
}

RD_factory::RD_factory(uint32_t num_samplers) noexcept
    : Factory(num_samplers), samplers_(memory::allocate_aligned<RD>(num_samplers)) {}

RD_factory::~RD_factory() noexcept {
    memory::free_aligned(samplers_);
}

Sampler* RD_factory::create(uint32_t id, rnd::Generator& rng) const noexcept {
    return new (&samplers_[id]) RD(rng);
}

}  // namespace sampler
