#include "sampler_uniform.hpp"
#include "camera_sample.hpp"
#include "base/math/vector.inl"
#include "base/random/generator.inl"

namespace sampler {

Uniform::Uniform(rnd::Generator& rng) : Sampler(rng) {}

void Uniform::generate_camera_sample(int2 pixel, uint32_t /*index*/, Camera_sample& sample) {
	float2 s2d(0.5f, 0.5f);

	sample.pixel = pixel;
	sample.pixel_uv = s2d;
	sample.lens_uv = s2d.yx();
	sample.time = rng_.random_float();
}

float2 Uniform::generate_sample_2D() {
	return float2(rng_.random_float(), rng_.random_float());
}

float Uniform::generate_sample_1D(uint32_t /*dimension*/) {
	return rng_.random_float();
}

void Uniform::on_resize() {}

void Uniform::on_resume_pixel(rnd::Generator& /*scramble*/) {}

Sampler* Uniform_factory::create(rnd::Generator& rng) const {
	return new Uniform(rng);
}

}
