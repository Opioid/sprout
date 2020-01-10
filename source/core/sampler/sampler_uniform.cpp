#include "sampler_uniform.hpp"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "camera_sample.hpp"
#include "sampler.inl"

namespace sampler {

Uniform::Uniform(rnd::Generator& rng) noexcept : Sampler(rng) {}

Camera_sample Uniform::generate_camera_sample(int2 pixel, uint32_t /*index*/) noexcept {
    float2 const s2d(0.5f, 0.5f);

    return Camera_sample{pixel, s2d, s2d.yx(), rng_.random_float()};
}

float2 Uniform::generate_sample_2D(uint32_t /*dimension*/) noexcept {
    return float2(rng_.random_float(), rng_.random_float());
}

float Uniform::generate_sample_1D(uint32_t /*dimension*/) noexcept {
    return rng_.random_float();
}

void Uniform::on_resize() noexcept {}

void Uniform::on_start_pixel() noexcept {}

template class Typed_pool<Uniform>;

}  // namespace sampler
