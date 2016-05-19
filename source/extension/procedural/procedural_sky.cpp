#include "procedural_sky.hpp"
#include "core/scene/scene_loader.hpp"

#include <iostream>

namespace procedural { namespace sky {

Provider provider;

void init(scene::Loader& loader) {
	loader.register_extension_provider("Sky", &provider);
}

scene::entity::Entity* Provider::create_extension(const json::Value& extension_value,
												  scene::Scene& scene,
												  resource::Manager& manager) {

	std::cout << "Sky provider not implemented" << std::endl;

	return nullptr;
}

}}
