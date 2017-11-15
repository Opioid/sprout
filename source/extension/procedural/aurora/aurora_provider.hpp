#ifndef SU_EXTENSION_PROCEDURAL_AURORA_PROVIDER_HPP
#define SU_EXTENSION_PROCEDURAL_AURORA_PROVIDER_HPP

#include "core/image/typed_image.hpp"
#include "core/scene/entity/entity_extension_provider.hpp"

namespace scene { class Loader; }

namespace procedural::aurora {

void init(scene::Loader& loader);

class Provider : public scene::entity::Extension_provider {

public:

	void set_scene_loader(scene::Loader& loader);

	virtual scene::entity::Entity* create_extension(const json::Value& extension_value,
													scene::Scene& scene,
													resource::Manager& manager) override final;

private:

	static void render(image::Byte3& target);

	scene::Loader* scene_loader_ = nullptr;
};

}

#endif
