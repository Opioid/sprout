#include "aurora_provider.hpp"
#include "aurora.hpp"
#include "volume_rasterizer.hpp"
#include "core/image/typed_image.inl"
#include "core/image/texture/texture_byte_3_srgb.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_loader.hpp"
#include "core/scene/volume/height.hpp"
#include "base/json/json.hpp"
#include "base/math/sampling/sample_distribution.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/vector3.inl"
#include "base/math/quaternion.inl"
#include "base/random/generator.inl"

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
	using namespace image;

	// const int3 dimensions(2048, 512, 256);
	const int3 dimensions(3072, 384, 192);

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
		float3(800000.f, 100000.f, 50000.f),
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


	const float start_height_deviation = 20000.f;
	const float start_height = ground_to_bottom_ + aurora_height_ - start_height_deviation;

	const uint32_t num_particles = 2 * 16384;

	const float nf = static_cast<float>(num_particles);

	for (uint32_t i = 0; i < num_particles; ++i) {
		const float cf = static_cast<float>(i);

		const float r0 = rng.random_float();

		const float x = math::radical_inverse_vdC(i, 0);// cf / nf;

		const float z = std::sin(x * (2.f * math::Pi));

		const float3 position(x * aurora_volume_[0],
							  start_height + r0 * start_height_deviation,
							  0.5f * aurora_volume_[2] + z * 50000.f);



		simulate_particle(position, rng, renderer);
	}


	renderer.resolve(target);
}

void Provider::simulate_particle(const float3& start, rnd::Generator& rng,
								 Volume_rasterizer& renderer) const {
	constexpr float step_size = 500.f;

	float3 color(0.025f, 0.05f, 0.0375f);

	float3 position = start;
	for (; position[1] > ground_to_bottom_;) {
		renderer.splat(world_to_grid(position), color);

		color += float3(0.0001f, 0.005f, 0.003f);

		position[0] += (1.f - 2.f * rng.random_float()) * 500.f;
		position[2] += (1.f - 2.f * rng.random_float()) * 500.f;

		position[1] -= step_size;
	}


}

float3 Provider::world_to_grid(const float3& position) const {
	return voxel_ratio_ * float3(position[0],
								 (ground_to_bottom_ + aurora_height_) - position[1],
								 position[2]);
}

}
