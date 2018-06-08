#include "sampler_uniform.hpp"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "camera_sample.hpp"

namespace sampler {

Uniform::Uniform(rnd::Generator& rng) : Sampler(rng) {}

void Uniform::generate_camera_sample(int2 pixel, uint32_t /*index*/, Camera_sample& sample) {
    float2 s2d(0.5f, 0.5f);

    sample.pixel    = pixel;
    sample.pixel_uv = s2d;
    sample.lens_uv  = s2d.yx();
    sample.time     = rng_.random_float();
}

float2 Uniform::generate_sample_2D(uint32_t /*dimension*/) {
    return float2(rng_.random_float(), rng_.random_float());
}

float Uniform::generate_sample_1D(uint32_t /*dimension*/) {
    return rng_.random_float();
}

size_t Uniform::num_bytes() const {
    return sizeof(*this);
}

void Uniform::on_resize() {}

void Uniform::on_resume_pixel(rnd::Generator& /*scramble*/) {}

Uniform_factory::Uniform_factory(uint32_t num_samplers)
    : Factory(num_samplers), samplers_(memory::allocate_aligned<Uniform>(num_samplers)) {}

Uniform_factory::~Uniform_factory() {
    memory::free_aligned(samplers_);
}

Sampler* Uniform_factory::create(uint32_t id, rnd::Generator& rng) const {
    return new (&samplers_[id]) Uniform(rng);
}

}  // namespace sampler
