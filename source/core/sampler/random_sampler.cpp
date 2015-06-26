#include "random_sampler.hpp"
#include "camera_sample.hpp"
#include "base/math/vector.inl"
#include "base/math/random/generator.inl"

namespace sampler {

Random::Random(math::random::Generator& rng, uint32_t num_samples_per_iteration) :
	Sampler(rng, num_samples_per_iteration) {}

Sampler* Random::clone() const {
	return new Random(rng_, num_samples_per_iteration_);
}

bool Random::generate_camera_sample(const math::float2& offset, Camera_sample& sample) {
	if (current_sample_ >= num_samples_per_iteration_) {
		return false;
	}

	math::float2 s2d(rng_.random_float(), rng_.random_float());

	sample.coordinates = offset + s2d;
	sample.relative_offset = s2d - math::float2(0.5f, 0.5f);
	sample.lens_uv = s2d.yx();
	sample.time = rng_.random_float();

	++current_sample_;

	return true;
}

math::float2 Random::generate_sample_2d() {
	return math::float2(rng_.random_float(), rng_.random_float());
}

float Random::generate_sample_1d() {
	return rng_.random_float();
}

}
