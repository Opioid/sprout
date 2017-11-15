#include "aurora_provider.hpp"
#include "aurora.hpp"
#include "volume_rasterizer.hpp"
#include "core/image/typed_image.inl"
#include "core/image/texture/texture_byte_3_srgb.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_loader.hpp"
#include "core/scene/volume/height.hpp"
#include "base/json/json.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/vector3.inl"
#include "base/math/quaternion.inl"

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

	const int3 dimensions(16);

	auto target = std::make_shared<Byte3>(Image::Description(Image::Type::Byte3, dimensions));

	Volume_rasterizer renderer(dimensions);

	renderer.clear();

	renderer.draw_sphere(float3(2.f, 2.f, 2.f), 1.f);

	renderer.resolve(*target);

	auto texture = std::make_shared<texture::Byte3_sRGB>(target);




	volume::Volume* volume = scene.create_grid_volume(texture);
	// volume::Volume* volume = scene.create_height_volume();

	constexpr char const* parameters =
		"{ \"scattering\": [1.0, 1.0, 1.0] }";

	volume->set_parameters(*json::parse(parameters));

	math::Transformation transformation {
		float3::identity(),
		float3(1.f),
		math::quaternion::identity()
	};

	volume->set_transformation(transformation);

	Aurora* aurora = new Aurora();

	aurora->attach(volume);

	return aurora;
}

}
