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

void Random::generate_camera_sample(math::float2 offset, uint32_t /*index*/, Camera_sample& sample) {
	math::float2 s2d(rng_.random_float(), rng_.random_float());

	sample.coordinates = offset + s2d;
	sample.relative_offset = s2d - math::float2(0.5f, 0.5f);
	sample.lens_uv = s2d.yx();
	sample.time = rng_.random_float();
}

math::float2 Random::generate_sample_2D() {
	return math::float2(rng_.random_float(), rng_.random_float());
}

float Random::generate_sample_1D() {
	return rng_.random_float();
}

math::uint2 Random::seed() const {
	return math::uint2::identity;
}

}
