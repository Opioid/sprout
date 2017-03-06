#include "sampler_ld.hpp"
#include "camera_sample.hpp"
#include "base/math/vector3.inl"
#include "base/math/sampling/sample_distribution.inl"
#include "base/random/generator.inl"

namespace sampler {

inline float2 ld(uint32_t i, uint32_t r0, uint32_t r1) {
	return float2(math::radical_inverse_vdC(i, r0),
				  math::radical_inverse_S(i, r1));
}

LD::LD(rnd::Generator& rng) : Sampler(rng) {}

void LD::generate_camera_sample(int2 pixel, uint32_t index, Camera_sample& sample) {
	float2 s2d = ld(index, scramble_[0], scramble_[1]);

	sample.pixel = pixel;
	sample.pixel_uv = s2d;
	sample.lens_uv = s2d.yx();
	sample.time = rng_.random_float();
}

float2 LD::generate_sample_2D(uint32_t /*dimension*/) {
	return ld(current_sample_2D_[0]++, scramble_[0], scramble_[1]);
}

float LD::generate_sample_1D(uint32_t /*dimension*/) {
	return rng_.random_float();
}

size_t LD::num_bytes() const {
	return sizeof(*this);
}

void LD::on_resize() {}

void LD::on_resume_pixel(rnd::Generator& scramble) {
	scramble_ = uint2(scramble.random_uint(), scramble.random_uint());
}

Sampler* LD_factory::create(rnd::Generator& rng) const {
	return new LD(rng);
}

}

