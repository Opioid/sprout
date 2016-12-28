#include "sampler_test.hpp"
#include "sampler_ems.hpp"
#include "sampler_golden_ratio.hpp"
#include "sampler_random.hpp"
#include "sampler_hammersley.hpp"
#include "image/typed_image.inl"
#include "image/encoding/png/png_writer.hpp"
#include "image/procedural/image_renderer.hpp"
#include "base/math/vector.inl"
#include "base/math/sampling/sampling.inl"
#include "base/random/generator.inl"
#include "base/spectrum/rgb.inl"

#include <iostream>
#include <string>
#include "base/math/print.hpp"

namespace sampler { namespace testing {

void render_set(const std::string& name, sampler::Sampler& sampler,
				image::procedural::Renderer& renderer, image::Byte_3& target);

void render_disk(const std::string& name, sampler::Sampler& sampler, rnd::Generator& scramble,
				 image::procedural::Renderer& renderer, image::Byte_3& target);

void render_quad(const std::string& name, sampler::Sampler& sampler, rnd::Generator& scramble,
				 image::procedural::Renderer& renderer, image::Byte_3& target);

void test() {
	std::cout << "sampler::testing::test()" << std::endl;

	int2 dimensions(512, 512);
	image::procedural::Renderer renderer(dimensions, 4);
	image::Byte_3 target(image::Image::Description(image::Image::Type::Byte_3, dimensions));

	uint32_t num_samples = 64;

	{
		rnd::Generator rng(6783452, 456679345, 347834, 56745234);
		sampler::EMS sampler(rng);
		sampler.resize(num_samples, 1, 1, 1);
		render_set("ems", sampler, renderer, target);
	}
	{
		rnd::Generator rng(6783452, 456679345, 347834, 56745234);
		sampler::Golden_ratio sampler(rng);
		sampler.resize(num_samples, 1, 1, 1);
		render_set("golden_ratio", sampler, renderer, target);
	}
	{
		rnd::Generator rng(6783452, 456679345, 347834, 56745234);
		sampler::Random sampler(rng);
		sampler.resize(num_samples, 1, 1, 1);
		render_set("random_disk", sampler, renderer, target);
	}
	{
		rnd::Generator rng(6783452, 456679345, 347834, 56745234);
		sampler::Hammersley sampler(rng);
		sampler.resize(num_samples, 1, 1, 1);
		render_set("hammersley", sampler, renderer, target);
	}
}

void render_set(const std::string& name, sampler::Sampler& sampler,
				image::procedural::Renderer& renderer, image::Byte_3& target) {
	rnd::Generator rng(68413684, 523489461, 318743219, 98765123);

	render_disk(name + "_disk_0.png", sampler, rng, renderer, target);
	render_disk(name + "_disk_1.png", sampler, rng, renderer, target);

	render_quad(name + "_quad_0.png", sampler, rng, renderer, target);
	render_quad(name + "_quad_1.png", sampler, rng, renderer, target);
}

void render_disk(const std::string& name, sampler::Sampler& sampler, rnd::Generator& scramble,
				 image::procedural::Renderer& renderer, image::Byte_3& target) {

	std::cout << name << ": ";

	const float2 center(0.5f, 0.5f);

	renderer.set_brush(float3(0.18f));
	renderer.clear();

	uint32_t num_samples = sampler.num_samples();

	uint32_t segment_len = num_samples / 4;

	float n = 1.f / static_cast<float>(segment_len);


	sampler.resume_pixel(0, scramble);

	renderer.set_brush(float3(1.f, 0.f, 0.f));

	float2 average(0.f);
	for (uint32_t i = 0; i < segment_len; ++i) {
		float2 s = sampler.generate_sample_2D();
		float2 ds = 0.5f * (math::sample_disk_concentric(s) + float2(1.f));
		average += n * ds;
		renderer.draw_circle(ds, 0.005f);
	}

	std::cout << math::distance(average, center) << ", ";

	renderer.set_brush(float3(0.f, 0.7f, 0.f));

	average = float2(0.f);
	for (uint32_t i = 0; i < segment_len; ++i) {
		float2 s = sampler.generate_sample_2D();
		float2 ds = 0.5f * (math::sample_disk_concentric(s) + float2(1.f));
		average += n * ds;
		renderer.draw_circle(ds, 0.005f);
	}

	std::cout << math::distance(average, center) << ", ";

	renderer.set_brush(float3(0.f, 0.f, 1.f));

	average = float2(0.f);
	for (uint32_t i = 0; i < segment_len; ++i) {
		float2 s = sampler.generate_sample_2D();
		float2 ds = 0.5f * (math::sample_disk_concentric(s) + float2(1.f));
		average += n * ds;
		renderer.draw_circle(ds, 0.005f);
	}

	std::cout << math::distance(average, center) << ", ";

	renderer.set_brush(float3(0.7f, 0.7f, 0.f));

	average = float2(0.f);
	for (uint32_t i = 0, len = num_samples / 4; i < len; ++i) {
		float2 s = sampler.generate_sample_2D();
		float2 ds = 0.5f * (math::sample_disk_concentric(s) + float2(1.f));
		average += n * ds;
		renderer.draw_circle(ds, 0.005f);
	}

	std::cout << math::distance(average, center) << std::endl;

	renderer.resolve_sRGB(target);

	image::encoding::png::Writer::write(name, target);
}

void render_quad(const std::string& name, sampler::Sampler& sampler, rnd::Generator& scramble,
				 image::procedural::Renderer& renderer, image::Byte_3& target) {
	renderer.set_brush(float3(0.18f));
	renderer.clear();

	uint32_t num_samples = sampler.num_samples();

	uint32_t segment_len = num_samples / 4;

	sampler.resume_pixel(0, scramble);

	renderer.set_brush(float3(1.f, 0.f, 0.f));
	for (uint32_t i = 0; i < segment_len; ++i) {
		float2 s = sampler.generate_sample_2D();
		renderer.draw_circle(s, 0.005f);
	}

	renderer.set_brush(float3(0.f, 0.7f, 0.f));
	for (uint32_t i = 0; i < segment_len; ++i) {
		float2 s = sampler.generate_sample_2D();
		renderer.draw_circle(s, 0.005f);
	}

	renderer.set_brush(float3(0.f, 0.f, 1.f));
	for (uint32_t i = 0; i < segment_len; ++i) {
		float2 s = sampler.generate_sample_2D();
		renderer.draw_circle(s, 0.005f);
	}

	renderer.set_brush(float3(0.7f, 0.7f, 0.f));
	for (uint32_t i = 0; i < segment_len; ++i) {
		float2 s = sampler.generate_sample_2D();
		renderer.draw_circle(s, 0.005f);
	}

	renderer.resolve_sRGB(target);

	image::encoding::png::Writer::write(name, target);
}

}}
