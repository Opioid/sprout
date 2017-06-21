#include "sampler_hammersley.hpp"
#include "camera_sample.hpp"
#include "base/math/vector3.inl"
#include "base/math/sampling/sample_distribution.hpp"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"

namespace sampler {

Hammersley::Hammersley(rnd::Generator& rng) : Sampler(rng) {}

void Hammersley::generate_camera_sample(int2 pixel, uint32_t index, Camera_sample& sample) {
	float2 s2d = math::hammersley(index, num_samples_, scramble_);

	sample.pixel = pixel;
	sample.pixel_uv = s2d;
	sample.lens_uv = s2d.yx();
	sample.time = rng_.random_float();
}

float2 Hammersley::generate_sample_2D(uint32_t /*dimension*/) {
	return math::hammersley(current_sample_2D_[0]++, num_samples_, scramble_);
}

float Hammersley::generate_sample_1D(uint32_t /*dimension*/) {
	return rng_.random_float();
}

size_t Hammersley::num_bytes() const {
	return sizeof(*this);
}

void Hammersley::on_resize() {}

void Hammersley::on_resume_pixel(rnd::Generator& scramble) {
	scramble_ = scramble.random_uint();
}

Hammersley_factory::Hammersley_factory(uint32_t num_samplers) :
	Factory(num_samplers),
	samplers_(memory::allocate_aligned<Hammersley>(num_samplers)) {}

Hammersley_factory::~Hammersley_factory() {
	memory::free_aligned(samplers_);
}

Sampler* Hammersley_factory::create(uint32_t id, rnd::Generator& rng) const {
	return new(&samplers_[id]) Hammersley(rng);
}

}
