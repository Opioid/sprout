#include "sampler_random.hpp"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "camera_sample.hpp"

namespace sampler {

Random::Random(rnd::Generator& rng) noexcept : Sampler(rng) {}

Camera_sample Random::generate_camera_sample(int2 pixel, uint32_t /*index*/) noexcept {
    return Camera_sample{pixel, float2(rng_.random_float(), rng_.random_float()),
                         float2(rng_.random_float(), rng_.random_float()), rng_.random_float()};
}

float2 Random::generate_sample_2D(uint32_t /*dimension*/) noexcept {
    return float2(rng_.random_float(), rng_.random_float());
}

float Random::generate_sample_1D(uint32_t /*dimension*/) noexcept {
    return rng_.random_float();
}

void Random::on_resize() noexcept {}

void Random::on_start_pixel() noexcept {}

}  // namespace sampler
