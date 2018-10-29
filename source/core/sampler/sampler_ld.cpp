#include "sampler_ld.hpp"
#include "base/math/sample_distribution.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "camera_sample.hpp"

namespace sampler {

inline float2 ld(uint32_t i, uint32_t r0, uint32_t r1) noexcept {
    return float2(math::radical_inverse_vdC(i, r0), math::radical_inverse_S(i, r1));
}

LD::LD(rnd::Generator& rng) noexcept : Sampler(rng) {}

Camera_sample LD::generate_camera_sample(int2 pixel, uint32_t index) noexcept {
    float2 s2d = ld(index, scramble_[0], scramble_[1]);

    return Camera_sample{pixel, s2d, s2d.yx(), rng_.random_float()};
}

float2 LD::generate_sample_2D(uint32_t /*dimension*/) noexcept {
    return ld(current_sample_2D_[0]++, scramble_[0], scramble_[1]);
}

float LD::generate_sample_1D(uint32_t /*dimension*/) noexcept {
    return rng_.random_float();
}

size_t LD::num_bytes() const noexcept {
    return sizeof(*this);
}

void LD::on_resize() noexcept {}

void LD::on_start_pixel() noexcept {
    scramble_ = uint2(rng_.random_uint(), rng_.random_uint());
}

LD_factory::LD_factory(uint32_t num_samplers) noexcept
    : Factory(num_samplers), samplers_(memory::allocate_aligned<LD>(num_samplers)) {}

LD_factory::~LD_factory() noexcept {
    memory::free_aligned(samplers_);
}

Sampler* LD_factory::create(uint32_t id, rnd::Generator& rng) const noexcept {
    return new (&samplers_[id]) LD(rng);
}

}  // namespace sampler
