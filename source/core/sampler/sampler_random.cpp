#include "sampler_random.hpp"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "sampler.inl"

namespace sampler {

Random::Random() = default;

Random::Random(uint32_t /*num_dimensions_2D*/, uint32_t /*num_dimensions_1D*/,
               uint32_t /*max_samples*/) {}

float2 Random::sample_2D(RNG& rng, uint32_t /*dimension*/) {
    return float2(rng.random_float(), rng.random_float());
}

float Random::sample_1D(RNG& rng, uint32_t /*dimension*/) {
    return rng.random_float();
}

template class Typed_pool<Random>;

}  // namespace sampler
