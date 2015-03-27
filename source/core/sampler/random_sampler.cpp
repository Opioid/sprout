#include "random_sampler.hpp"
#include "camera_sample.hpp"
#include "base/math/vector.inl"
#include "base/math/random/generator.inl"

namespace sampler {

Random::Random(uint32_t num_samples_per_iteration, math::random::Generator& rng) :
	Sampler(num_samples_per_iteration), rng_(rng) {}

Sampler* Random::clone(math::random::Generator& rng) const {
	return new Random(num_samples_per_iteration_, rng);
}

bool Random::generate_camera_sample(const math::float2& offset, Camera_sample& sample) {
	if (current_sample_ >= num_samples_per_iteration_) {
		return false;
	}

	math::float2 s2d(rng_.random_float(), rng_.random_float());

	sample.coordinates = offset + s2d;
	sample.relative_offset = s2d - math::float2(0.5f, 0.5f);
	sample.lens_uv = s2d.yx();

	++current_sample_;

	return true;
}

math::float2 Random::generate_sample2d() {
	return math::float2(rng_.random_float(), rng_.random_float());
}

}
