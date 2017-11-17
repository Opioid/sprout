#include "aurora_provider.hpp"
#include "aurora.hpp"
#include "aurora_spectrum.hpp"
#include "volume_rasterizer.hpp"
#include "core/image/typed_image.inl"
#include "core/image/texture/texture_byte_3_srgb.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_loader.hpp"
#include "core/scene/volume/height.hpp"
#include "base/json/json.hpp"
#include "base/math/sampling/sampling.hpp"
#include "base/math/sampling/sample_distribution.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/vector3.inl"
#include "base/math/quaternion.inl"
#include "base/random/generator.inl"

#include <iostream>

namespace procedural::aurora {

using namespace scene;

Provider provider;

void init(scene::Loader& loader) {
	provider.set_scene_loader(loader);

	loader.register_extension_provider("Aurora", &provider);


}

void Provider::set_scene_loader(Loader& loader) {
	scene_loader_ = &loader;
}

entity::Entity* Provider::create_extension(const json::Value& extension_value,
										   Scene& scene, resource::Manager& /*manager*/) {
	spectrum::init();

	using namespace image;

	// const int3 dimensions(2048, 512, 256);
	// const int3 dimensions(3072, 384, 192);
//	const int3 dimensions(3200, 320, 160);
	const int3 dimensions(3200, 320, 256);

	auto target = std::make_shared<Byte3>(Image::Description(Image::Type::Byte3, dimensions));

	render(*target);

	auto texture = std::make_shared<texture::Byte3_sRGB>(target);

	volume::Volume* volume = scene.create_grid_volume(texture);
	//volume::Volume* volume = scene.create_height_volume();

	constexpr char const* parameters =
		"{ \"scattering\": [1.0, 1.0, 1.0] }";

	volume->set_parameters(*json::parse(parameters));

	math::Transformation transformation {
		float3::identity(),
	//	float3(1000000.f, 100000.f, 50000.f),
		float3(1000000.f, 100000.f, 80000.f),
		math::quaternion::identity()
	};

	volume->set_transformation(transformation);

	Aurora* aurora = new Aurora();

	aurora->attach(volume);

	return aurora;
}

void Provider::render(image::Byte3& target) {
	rnd::Generator rng;

	Volume_rasterizer renderer(target.description().dimensions);

	const float3 dimensions(target.description().dimensions);

	voxel_ratio_ = dimensions[1] / aurora_height_;

	aurora_volume_ = float3(dimensions[0] / voxel_ratio_,
							aurora_height_,
							dimensions[2] / voxel_ratio_);

	renderer.set_brush(float3(0.f));
	renderer.clear();


	const float start_height_deviation = 60000.f;
	const float start_height = ground_to_bottom_ + aurora_height_ - start_height_deviation;

	const uint32_t num_particles = 256 * 16384;

	const float nf = static_cast<float>(num_particles);

	for (uint32_t i = 0; i < num_particles; ++i) {
		const float cf = static_cast<float>(i);

		const float r0 = rng.random_float();

		const float x = math::radical_inverse_vdC(i, 0);// cf / nf;

		const float z = std::sin(x * (2.f * math::Pi)) * 50000.f
					  + std::sin(3.5f * x * (2.f * math::Pi)) * 20000.f
					  + (x * x) * std::sin(17.f * x * (2.f * math::Pi)) * 10000.f;

		const float y = start_height + std::sin(x * (7.f * math::Pi)) * start_height_deviation;
	//	const float y = start_height + start_height_deviation;

		const float3 position(x * aurora_volume_[0],
							  ground_to_bottom_ + aurora_height_,
							  0.5f * aurora_volume_[2] + z);



		simulate_particle(position, y, rng, renderer);
	}


	renderer.resolve(target);
}

void Provider::simulate_particle(const float3& start, float peak_height, rnd::Generator& rng,
								 Volume_rasterizer& renderer) const {
	const float step_size = 1.f / voxel_ratio_;

	const float range = (start[1] - ground_to_bottom_);
	const float grace_range = (start[1] - peak_height);

	float3 position = start;
	for (; position[1] > ground_to_bottom_;) {
		const float normalized_height = (position[1] - ground_to_bottom_) / aurora_height_;

		const float progress = 1.f - (position[1] - ground_to_bottom_) / range;
		const float grace_progress = std::min(1.f - (position[1] - peak_height) / grace_range, 1.f);

		//	const float progress = 1.f - normalized_height;
		const float spread = (100.f + progress * 500.f);

		renderer.splat(world_to_grid(position),
					   progress * grace_progress * spectrum::linear_rgb(normalized_height));

		const float2 r2(rng.random_float(), rng.random_float());
		const float2 uv = math::sample_disk_concentric(r2);

		position[0] += uv[0] * spread;
		position[2] += uv[1] * spread;

		position[1] -= step_size;
	}


}

float3 Provider::world_to_grid(const float3& position) const {
	return voxel_ratio_ * float3(position[0],
								 (ground_to_bottom_ + aurora_height_) - position[1],
								 position[2]);
}

}
