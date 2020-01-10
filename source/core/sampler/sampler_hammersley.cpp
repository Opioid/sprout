#include "sampler_hammersley.hpp"
#include "base/math/sample_distribution.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "camera_sample.hpp"
#include "sampler.inl"

namespace sampler {

Hammersley::Hammersley(rnd::Generator& rng) noexcept : Sampler(rng) {}

Camera_sample Hammersley::generate_camera_sample(int2 pixel, uint32_t index) noexcept {
    float2 s2d = math::hammersley(index, num_samples_, scramble_);

    return Camera_sample{pixel, s2d, s2d.yx(), rng_.random_float()};
}

float2 Hammersley::generate_sample_2D(uint32_t /*dimension*/) noexcept {
    return math::hammersley(current_sample_2D_[0]++, num_samples_, scramble_);
}

float Hammersley::generate_sample_1D(uint32_t /*dimension*/) noexcept {
    return rng_.random_float();
}

void Hammersley::on_resize() noexcept {}

void Hammersley::on_start_pixel() noexcept {
    scramble_ = rng_.random_uint();
}

template class Typed_pool<Hammersley>;

}  // namespace sampler
