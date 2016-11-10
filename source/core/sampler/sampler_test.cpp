#include "sampler_test.hpp"
#include "sampler_ems.hpp"
#include "sampler_golden_ratio.hpp"
#include "sampler_halton.hpp"
#include "sampler_random.hpp"
#include "sampler_scrambled_hammersley.hpp"
#include "sampler_sobol.hpp"
#include "image/typed_image.inl"
#include "image/encoding/png/png_writer.hpp"
#include "image/procedural/image_renderer.hpp"
#include "base/math/vector.inl"
#include "base/math/sampling/sampling.inl"
#include "base/random/generator.inl"
#include "base/spectrum/rgb.inl"

#include <iostream>
#include "base/math/print.hpp"

namespace sampler { namespace testing {

void render_set(const std::string name, sampler::Sampler& sampler, uint2 seed,
				image::procedural::Renderer& renderer,
				image::Image_byte_3& target);

void render_disk(const std::string& name, sampler::Sampler& sampler, uint2 seed,
				 image::procedural::Renderer& renderer,
				 image::Image_byte_3& target);

void render_quad(const std::string& name, sampler::Sampler& sampler, uint2 seed,
				 image::procedural::Renderer& renderer,
				 image::Image_byte_3& target);

void test() {
	std::cout << "sampler::testing::test()" << std::endl;

	int2 dimensions(512, 512);
	image::procedural::Renderer renderer(dimensions, 4);
	image::Image_byte_3 target(image::Image::Description(image::Image::Type::Byte_3, dimensions));

	random::Generator rng(6783452, 456679345, 347834, 56745234);

	uint2 seed(rng.random_uint(), rng.random_uint());

	uint32_t num_samples = 64;

	{
		sampler::EMS sampler(rng, num_samples);
		render_set("ems", sampler, seed, renderer, target);
	}
	{
		sampler::Golden_ratio sampler(rng, num_samples);
		render_set("golden_ratio", sampler, seed, renderer, target);
	}
	{
		sampler::Halton sampler(rng, num_samples);
		render_set("halton", sampler, seed, renderer, target);
	}
	{
		sampler::Random sampler(rng, num_samples);
		render_set("random_disk", sampler, seed, renderer, target);
	}
	{
		sampler::Scrambled_hammersley sampler(rng, num_samples);
		render_set("scrambled_hammersley", sampler, seed, renderer, target);
	}
	{
		sampler::Sobol sampler(rng, num_samples);
		render_set("sobol_disk", sampler, seed, renderer, target);
	}
}

void render_set(const std::string name, sampler::Sampler& sampler, uint2 seed,
				image::procedural::Renderer& renderer,
				image::Image_byte_3& target) {
	render_disk(name + "_disk_0.png", sampler, uint2(0, 0), renderer, target);
	render_disk(name + "_disk_1.png", sampler, seed, renderer, target);

	render_quad(name + "_quad_0.png", sampler, uint2(0, 0), renderer, target);
	render_quad(name + "_quad_1.png", sampler, seed, renderer, target);
}

void render_disk(const std::string& name, sampler::Sampler& sampler, uint2 seed,
				 image::procedural::Renderer& renderer,
				 image::Image_byte_3& target) {
	renderer.set_brush(float3(0.18f));
	renderer.clear();

	uint32_t num_samples = sampler.num_samples();

	sampler.resume_pixel(0, seed);

	renderer.set_brush(float3(1.f, 0.f, 0.f));
	for (uint32_t i = 0, len = num_samples / 4; i < len; ++i) {
		float2 s = sampler.generate_sample_2D();
		float2 ds = 0.5f * (math::sample_disk_concentric(s) + float2(1.f));
		renderer.draw_circle(ds, 0.005f);
	}

	renderer.set_brush(float3(0.f, 0.7f, 0.f));
	for (uint32_t i = 0, len = num_samples / 4; i < len; ++i) {
		float2 s = sampler.generate_sample_2D();
		float2 ds = 0.5f * (math::sample_disk_concentric(s) + float2(1.f));
		renderer.draw_circle(ds, 0.005f);
	}

	renderer.set_brush(float3(0.f, 0.f, 1.f));
	for (uint32_t i = 0, len = num_samples / 4; i < len; ++i) {
		float2 s = sampler.generate_sample_2D();
		float2 ds = 0.5f * (math::sample_disk_concentric(s) + float2(1.f));
		renderer.draw_circle(ds, 0.005f);
	}

	renderer.set_brush(float3(0.7f, 0.7f, 0.f));
	for (uint32_t i = 0, len = num_samples / 4; i < len; ++i) {
		float2 s = sampler.generate_sample_2D();
		float2 ds = 0.5f * (math::sample_disk_concentric(s) + float2(1.f));
		renderer.draw_circle(ds, 0.005f);
	}

	renderer.resolve_sRGB(target);

	image::encoding::png::Writer::write(name, target);
}

void render_quad(const std::string& name, sampler::Sampler& sampler, uint2 seed,
				 image::procedural::Renderer& renderer,
				 image::Image_byte_3& target) {
	renderer.set_brush(float3(0.18f));
	renderer.clear();

	uint32_t num_samples = sampler.num_samples();

	sampler.resume_pixel(0, seed);

	renderer.set_brush(float3(1.f, 0.f, 0.f));
	for (uint32_t i = 0, len = num_samples / 4; i < len; ++i) {
		float2 s = sampler.generate_sample_2D();
		renderer.draw_circle(s, 0.005f);
	}

	renderer.set_brush(float3(0.f, 0.7f, 0.f));
	for (uint32_t i = 0, len = num_samples / 4; i < len; ++i) {
		float2 s = sampler.generate_sample_2D();
		renderer.draw_circle(s, 0.005f);
	}

	renderer.set_brush(float3(0.f, 0.f, 1.f));
	for (uint32_t i = 0, len = num_samples / 4; i < len; ++i) {
		float2 s = sampler.generate_sample_2D();
		renderer.draw_circle(s, 0.005f);
	}

	renderer.set_brush(float3(0.7f, 0.7f, 0.f));
	for (uint32_t i = 0, len = num_samples / 4; i < len; ++i) {
		float2 s = sampler.generate_sample_2D();
		renderer.draw_circle(s, 0.005f);
	}

	renderer.resolve_sRGB(target);

	image::encoding::png::Writer::write(name, target);
}

}}
