#include "sampler_hammersley.hpp"
#include "camera_sample.hpp"
#include "base/math/vector3.inl"
#include "base/math/sampling/sample_distribution.inl"
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

Sampler* Hammersley_factory::create(rnd::Generator& rng) const {
	return new Hammersley(rng);
}

}
