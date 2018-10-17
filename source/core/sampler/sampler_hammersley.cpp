#include "sampler_hammersley.hpp"
#include "base/math/sample_distribution.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "camera_sample.hpp"

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

size_t Hammersley::num_bytes() const noexcept {
    return sizeof(*this);
}

void Hammersley::on_resize() noexcept {}

void Hammersley::on_start_pixel() noexcept {
    scramble_ = rng_.random_uint();
}

Hammersley_factory::Hammersley_factory(uint32_t num_samplers) noexcept
    : Factory(num_samplers), samplers_(memory::allocate_aligned<Hammersley>(num_samplers)) {}

Hammersley_factory::~Hammersley_factory() noexcept {
    memory::free_aligned(samplers_);
}

Sampler* Hammersley_factory::create(uint32_t id, rnd::Generator& rng) const noexcept {
    return new (&samplers_[id]) Hammersley(rng);
}

}  // namespace sampler
