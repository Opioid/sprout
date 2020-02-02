#include "sampler_random.hpp"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "sampler.inl"

namespace sampler {

Random::Random(rnd::Generator& rng) : Sampler(rng) {}

float2 Random::generate_sample_2D(uint32_t /*dimension*/) {
    return float2(rng_.random_float(), rng_.random_float());
}

float Random::generate_sample_1D(uint32_t /*dimension*/) {
    return rng_.random_float();
}

void Random::on_resize() {}

void Random::on_start_pixel() {}

template class Typed_pool<Random>;

}  // namespace sampler
