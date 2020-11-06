#include "sampler_random.hpp"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "sampler.inl"

namespace sampler {

Random::Random() {}

float2 Random::sample_2D(RNG& rng, uint32_t /*dimension*/) {
    return float2(rng.random_float(), rng.random_float());
}

float Random::sample_1D(RNG& rng, uint32_t /*dimension*/) {
    return rng.random_float();
}

void Random::on_resize() {}

void Random::on_start_pixel(rnd::Generator& /*rng*/) {}

template class Typed_pool<Random>;

}  // namespace sampler
