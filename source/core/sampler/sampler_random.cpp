#include "sampler_random.hpp"
#include "camera_sample.hpp"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"

namespace sampler {

Random::Random(rnd::Generator& rng) : Sampler(rng) {}

void Random::generate_camera_sample(int2 pixel, uint32_t /*index*/,
									Camera_sample& sample) {
	sample.pixel = pixel;
	sample.pixel_uv = float2(rng_.random_float(), rng_.random_float());
	sample.lens_uv = float2(rng_.random_float(), rng_.random_float());
	sample.time = rng_.random_float();
}

float2 Random::generate_sample_2D(uint32_t /*dimension*/) {
	return float2(rng_.random_float(), rng_.random_float());
}

float Random::generate_sample_1D(uint32_t /*dimension*/) {
	return rng_.random_float();
}

size_t Random::num_bytes() const {
	return sizeof(*this);
}

void Random::on_resize() {}

void Random::on_resume_pixel(rnd::Generator& /*scramble*/) {}

Random_factory::Random_factory(uint32_t num_samplers) :
	Factory(num_samplers),
	samplers_(memory::allocate_aligned<Random>(num_samplers)) {}

Random_factory::~Random_factory() {
	memory::free_aligned(samplers_);
}

Sampler* Random_factory::create(uint32_t id, rnd::Generator& rng) const {
	return new(&samplers_[id]) Random(rng);
}

}
