#include "sampler_golden_ratio.hpp"
#include "camera_sample.hpp"
#include "base/math/vector.inl"
#include "base/math/random/generator.inl"
#include "base/math/sampling/sample_distribution.inl"
#include <random>

namespace sampler {

Golden_ratio::Golden_ratio(math::random::Generator& rng,
						   uint32_t num_samples) :
	Sampler(rng, num_samples) {}

void Golden_ratio::generate_camera_sample(int2 pixel, uint32_t index,
										  Camera_sample& sample) {
	float2 s2d = math::scrambled_hammersley(index, num_samples_, seed_.x);

	sample.pixel = pixel;
	sample.pixel_uv = s2d;
	sample.lens_uv = s2d.yx();
	sample.time = rng_.random_float();
}

float2 Golden_ratio::generate_sample_2D() {
	return samples_[current_sample_++];
}

float Golden_ratio::generate_sample_1D() {
	return rng_.random_float();
}

void Golden_ratio::on_resume_pixel() {
	uint32_t num_samples = num_samples_;

	samples_.resize(num_samples);

	float2 r(rng_.cast(seed_.x), rng_.cast(seed_.y));
	math::golden_ratio(samples_.data(), num_samples_, r);

//	std::random_device rd;
	std::mt19937 g(/*rd()*/seed_.x);
	std::shuffle(samples_.begin(), samples_.end(), g);
}

Golden_ratio_factory::Golden_ratio_factory(uint32_t num_samples) :
	Factory(num_samples) {}

Sampler* Golden_ratio_factory::create(math::random::Generator& rng) const {
	return new Golden_ratio(rng, num_samples_);
}

}
