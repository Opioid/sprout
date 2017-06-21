#include "sampler_ems.hpp"
#include "camera_sample.hpp"
#include "base/math/vector3.inl"
#include "base/math/sampling/sample_distribution.hpp"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"

namespace sampler {

EMS::EMS(rnd::Generator& rng) : Sampler(rng) {}

void EMS::generate_camera_sample(int2 pixel, uint32_t index, Camera_sample& sample) {
	float2 s2d = math::ems(index, scramble_[0], scramble_[1]);

	sample.pixel = pixel;
	sample.pixel_uv = s2d;
	sample.lens_uv = s2d.yx();
	sample.time = math::radical_inverse_vdC(index, scramble_[1]);
}

float2 EMS::generate_sample_2D(uint32_t /*dimension*/) {
	return math::ems(current_sample_2D_[0]++, scramble_[0], scramble_[1]);
}

float EMS::generate_sample_1D(uint32_t /*dimension*/) {
	return rng_.random_float();
//	return math::scrambled_radical_inverse_vdC(current_sample_++, seed_.y);
}

size_t EMS::num_bytes() const {
	return sizeof(*this);
}

void EMS::on_resize() {}

void EMS::on_resume_pixel(rnd::Generator& scramble) {
	scramble_ = uint2(scramble.random_uint(), scramble.random_uint());
}

EMS_factory::EMS_factory(uint32_t num_samplers) :
	Factory(num_samplers),
	samplers_(memory::allocate_aligned<EMS>(num_samplers)) {}

EMS_factory::~EMS_factory() {
	memory::free_aligned(samplers_);
}

Sampler* EMS_factory::create(uint32_t id, rnd::Generator& rng) const {
	return new(&samplers_[id]) EMS(rng);
}

}
