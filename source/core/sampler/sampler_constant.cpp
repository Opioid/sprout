#include "sampler_constant.hpp"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "camera_sample.hpp"

namespace sampler {

Constant::Constant(rnd::Generator& rng) : Sampler(rng) {}

void Constant::generate_camera_sample(int2 pixel, uint32_t /*index*/, Camera_sample& sample) {
    sample.pixel    = pixel;
    sample.pixel_uv = r2_;
    sample.lens_uv  = r2_;
    sample.time     = rng_.random_float();
}

float2 Constant::generate_sample_2D(uint32_t /*dimension*/) {
    return r2_;
}

float Constant::generate_sample_1D(uint32_t /*dimension*/) {
    return r_;
}

size_t Constant::num_bytes() const {
    return sizeof(*this);
}

void Constant::set(float2 r2) {
    r2_ = r2;
}

void Constant::set(float r) {
    r_ = r;
}

void Constant::on_resize() {}

void Constant::on_resume_pixel(rnd::Generator& /*scramble*/) {}

Constant_factory::Constant_factory(uint32_t num_samplers)
    : Factory(num_samplers), samplers_(memory::allocate_aligned<Constant>(num_samplers)) {}

Constant_factory::~Constant_factory() {
    memory::free_aligned(samplers_);
}

Sampler* Constant_factory::create(uint32_t id, rnd::Generator& rng) const {
    return new (&samplers_[id]) Constant(rng);
}

}  // namespace sampler
