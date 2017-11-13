#include "aurora_provider.hpp"
#include "core/scene/scene_loader.hpp"

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
										   Scene& scene, resource::Manager& manager) {
	return nullptr;
}

}
